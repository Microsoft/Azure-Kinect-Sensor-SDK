// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#ifndef DEPTH_EVAL_H
#define DEPTH_EVAL_H

#include <string>

void help();

static bool process_mkv(const std::string &passive_ir_mkv,
                        const std::string &depth_mkv,
                        const std::string &template_file,
                        int timestamp,
                        const std::string &output_dir,
                        float gray_gamma,
                        float gray_max,
                        float gray_percentile,
                        bool save_images);

#endif
