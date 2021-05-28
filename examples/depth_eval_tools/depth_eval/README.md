# Azure Kinect - Depth Evaluation Tools Examples - depth_eval

---

## Description

   Depth Evaluation Tool for K4A.

   This tool utilizes two mkv files.

   The 1st mkv file is PASSIVE_IR recorded using: ```k4arecorder.exe -c 3072p -d PASSIVE_IR -l 3  board1.mkv```

   The 2nd mkv file is WFOV_2X2BINNED recorded using: ```k4arecorder.exe -c 3072p -d WFOV_2X2BINNED -l 3 board2.mkv```

   This version supports WFOV_2X2BINNED but can be easily generalized.

---

## Usage

   ```
   ./depth_eval -h or -help or -? print the help message
   ./depth_eval -i=<passive_ir mkv file> -d=<depth mkv file> -t=<board json template>
   -out=<output directory> -s=<1:generate and save result images>
   -gg=<gray_gamma used to convert ir data to 8bit gray image. default=0.5>
   -gm=<gray_max used to convert ir data to 8bit gray image. default=4000.0>
   -gp=<percentile used to convert ir data to 8bit gray image. default=99.0>
   ```

   Example Command: ```./depth_eval -i=board1.mkv -d=board2.mkv -t=plane.json -out=c:/data```

---
## Dependencies 

   OpenCV
   OpenCV Contrib
