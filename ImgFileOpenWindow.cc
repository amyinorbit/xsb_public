/*
 * CDDL HEADER START
 *
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 *
 * CDDL HEADER END
*/
/*
 * Copyright 2021 Saso Kiselkov. All rights reserved.
 */

#include <algorithm>

#include <acfutils/assert.h>
#include <acfutils/helpers.h>

#include "ImgFileOpenWindow.h"

#define	WIN_WIDTH	600
#define	WIN_HEIGHT	600

static bool
match_extension(const char *file_ext, const char *match_ext)
{
	ASSERT(match_ext != NULL);
	if (strcmp(match_ext, "*") == 0 || strcmp(match_ext, "") == 0)
		return (true);
	if (file_ext == NULL)
		return (false);
	return (strcmp(file_ext, match_ext) == 0);
}

ImgFileOpenWindow::ImgFileOpenWindow(std::string title, std::string dirpath,
    std::string exts, bool save_window, file_win_done_cb_t done_cb,
    void *userinfo) :
    ImgWindow(100, WIN_HEIGHT + 100, WIN_WIDTH + 100, 100,
    xplm_WindowDecorationRoundRectangle, xplm_WindowLayerModal)
{
	ASSERT(done_cb != NULL);

	save = save_window;
	this->extensions = exts;
	this->dirpath = dirpath;
	this->done_cb = done_cb;
	this->userinfo = userinfo;
	delay_line_init(&refresh_intval, SEC2USEC(1));
	delay_line_push_imm_u64(&refresh_intval, 1);
	SetWindowTitle(title);
}

void
ImgFileOpenWindow::confirm(void)
{
	char *path;
	bool done = false;

	if (sel_filename.compare("") == 0)
		return;

	path = mkpathname(dirpath.c_str(), sel_filename.c_str(), NULL);
	if (save) {
		if (!file_exists(path, NULL) || this->confirm_overwrite)
			done = true;
		else
			this->confirm_overwrite = true;
	} else {
		done = true;
	}
	if (done) {
		done_cb(std::string(path), userinfo);
		SetVisible(false);
		this->sel_filename = "";
		this->confirm_overwrite = false;
	}
	LACF_DESTROY(path);
}

void
ImgFileOpenWindow::buildInterface(void)
{
	char buf[256];

	refresh();

	if (ImGui::BeginChild("dirlist", ImVec2(0, WIN_HEIGHT - 65))) {
		 for (std::vector<std::string>::iterator it = filenames.begin();
		    it != filenames.end(); it++) {
			auto filename = *it;
			bool is_sel = (filename.compare(this->sel_filename) ==
			    0);

			if (ImGui::Selectable(filename.c_str(), is_sel))
				sel_filename = std::string(filename);
			if (is_sel)
				ImGui::SetItemDefaultFocus();
		}
	}
	ImGui::EndChild();
	ImGui::PushItemWidth(WIN_WIDTH - 20);
	strlcpy(buf, this->sel_filename.c_str(), sizeof (buf));
	ImGui::InputText("##filename", buf, sizeof (buf));
	if (this->sel_filename.compare(std::string(buf)) != 0) {
		this->sel_filename = std::string(buf);
		this->confirm_overwrite = false;
	}
	ImGui::PopItemWidth();

	ImGui::Dummy(ImVec2(WIN_WIDTH - 178, 1));
	ImGui::SameLine();
	if (confirm_overwrite) {
		if (ImGui::Button("Overwrite?"))
			confirm();
	} else if (ImGui::Button(save ? "   Save   " : "   Open   ")) {
		confirm();
	}
	ImGui::SameLine();
	if (ImGui::Button(" Cancel ")) {
		SetVisible(false);
		sel_filename = "";
		confirm_overwrite = false;
	}
}

static bool
icomp(const std::string& str1, const std::string& str2)
{
	std::string str1_copy(str1);
	std::string str2_copy(str2);
	std::transform(str1_copy.begin(), str1_copy.end(), str1_copy.begin(),
	    ::tolower);
	std::transform(str2_copy.begin(), str2_copy.end(), str2_copy.begin(),
	    ::tolower);
	return (str1_copy.compare(str2_copy) < 0);
}

void
ImgFileOpenWindow::refresh(void)
{
	size_t n_exts;
	char **exts;
	DIR *dp;
	struct dirent *de;

	if (!delay_line_push_u64(&refresh_intval, 1))
		return;
	delay_line_push_imm_u64(&refresh_intval, 0);

	filenames.clear();

	exts = strsplit(extensions.c_str(), ";", true, &n_exts);
	dp = opendir(dirpath.c_str());
	if (dp == NULL)
		return;

	while ((de = readdir(dp)) != NULL) {
		char name[sizeof (de->d_name)];
		const char *ext;

		strlcpy(name, de->d_name, sizeof (name));
		strtolower(name);

		ext = strrchr(name, '.');
		if (ext != NULL)
			ext++;
		for (size_t i = 0; i < n_exts; i++) {
			if (match_extension(ext, exts[i]))
				filenames.push_back(std::string(de->d_name));
		}
	}
	closedir(dp);
	free_strlist(exts, n_exts);

	std::sort(filenames.begin(), filenames.end(), icomp);
}
