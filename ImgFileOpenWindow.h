/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license in the file COPYING
 * or http://www.opensource.org/licenses/CDDL-1.0.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file COPYING.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2021 Saso Kiselkov. All rights reserved.
 */

#ifndef	_IMG_FILE_OPEN_WINDOW_H_
#define	_IMG_FILE_OPEN_WINDOW_H_

#include <string>
#include <vector>

#if	!IBM
#include <sys/stat.h>
#include <sys/types.h>
#endif	/* !IBM */

#include <acfutils/core.h>
#include <acfutils/delay_line.h>
#include <acfutils/helpers.h>
#include <acfutils/stat.h>

#include "ImgWindow.h"

typedef void (*file_win_done_cb_t)(std::string path, void *userinfo);

typedef struct {
	std::string	filename;
	struct stat	st;
} file_info_t;

class ImgFileOpenWindow : public ImgWindow {
	bool				save;
	bool				confirm_overwrite = false;
	delay_line_t			refresh_intval;
	std::string			sel_filename = "";
	std::vector<file_info_t>	files;
	std::string			extensions;
	std::string			dirpath;
	file_win_done_cb_t		done_cb;
	void				*userinfo;
	uint64_t			last_click_t = 0;

	void buildInterface(void);
	void confirm(void);
	void refresh(void);
public:
	ImgFileOpenWindow(std::string title, std::string dirpath,
	    std::string exts, bool save_window, bool modal,
	    file_win_done_cb_t done_cb, void *userinfo);
};

#endif	/* _IMG_FILE_OPEN_WINDOW_H_ */
