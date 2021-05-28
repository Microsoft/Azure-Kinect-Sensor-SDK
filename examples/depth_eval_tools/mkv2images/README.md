# Azure Kinect - Depth Evaluation Tools Examples - mkv2images

---
## Description

   Dump mkv file to png images.

---
## Usage

   ```
   ./mkv2images -h or -help or -? print the help message
   ./mkv2images -in=<input mkv file> -out=<output directory> -d=<dump depth> -i=<dump ir> -c=<dump color>
   -f=<0:dump mean images only, 1 : dump first frame>
   -gg=<gray_gamma used to convert ir data to 8bit gray image. default=0.5>
   -gm=<gray_max used to convert ir data to 8bit gray image. default=4000.0>
   -gp=<percentile used to convert ir data to 8bit gray image. default=99.0>
   ```

   Example Command: ```./mkv2images -in=board1.mkv -out=c:/data -c=0 -f=0```

---

## Dependencies 

   OpenCV
   OpenCV Contrib
