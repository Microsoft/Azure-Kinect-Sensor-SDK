# Azure Kinect - Depth Evaluation Tools Examples - collect

---

## Description

   Collect multiple view, depth and color images from K4A device and pre-process the data for use with the other evaluation tools.

---

## Usage
   ```
   ./collect -h or -help or -? print the help message
   ./collect -mode=<depth mode> -res=<color resolution> -nv=<num of views> -nc=<num of captures per view>
   -fps=<frame rate enum> -cal=<dump cal file> -xy=<dump xytable> -d=<capture depth> -i=<capture ir> -c=<capture color>
   -out=<output directory>
   -gg=<gray_gamma used to convert ir data to 8bit gray image. default=0.5>
   -gm=<gray_max used to convert ir data to 8bit gray image. default=4000.0>
   -gp=<percentile used to convert ir data to 8bit gray image. default=99.0>
   -av=<0:dump mean images only, 1:dump all images, 2:dump all images and their mean>
   ```

   Example Command: ```./collect  -mode=3 -res=1 -nv=2 -nc=10 -cal=1 -out=c:/data```

   ---
   ---  

   Depth mode can be [0, 1, 2, 3, 4 or 5] as follows:

   | Depth Mode | Details |
   | :--------- | :------ |
   | K4A_DEPTH_MODE_OFF = 0 | 0:Depth sensor will be turned off with this setting. |
   | K4A_DEPTH_MODE_NFOV_2X2BINNED | 1:Depth captured at 320x288. Passive IR is also captured at 320x288. |
   | K4A_DEPTH_MODE_NFOV_UNBINNED | 2:Depth captured at 640x576. Passive IR is also captured at 640x576. |
   | K4A_DEPTH_MODE_WFOV_2X2BINNED | 3:Depth captured at 512x512. Passive IR is also captured at 512x512. |
   | K4A_DEPTH_MODE_WFOV_UNBINNED | 4:Depth captured at 1024x1024. Passive IR is also captured at 1024x1024. |
   | K4A_DEPTH_MODE_PASSIVE_IR | 5:Passive IR only, captured at 1024x1024. |

   ---
   ---

   Color resolution can be [0, 1, 2, 3, 4, 5, or 6] as follows:

   | Color Resolution | Details |
   | :--------------- | :------ |
   | K4A_COLOR_RESOLUTION_OFF = 0 | 0: Color camera will be turned off. |
   | K4A_COLOR_RESOLUTION_720P | 1: 1280 * 720  16:9. |
   | K4A_COLOR_RESOLUTION_1080P | 2: 1920 * 1080 16:9. |
   | K4A_COLOR_RESOLUTION_1440P | 3: 2560 * 1440 16:9. |
   | K4A_COLOR_RESOLUTION_1536P | 4: 2048 * 1536 4:3. |
   | K4A_COLOR_RESOLUTION_2160P | 5: 3840 * 2160 16:9. |
   | K4A_COLOR_RESOLUTION_3072P | 6: 4096 * 3072 4:3. |

   ---
   ---

   FPS can be [0, 1, or 2] as follows:

   | FPS Mode | Details |
   | :------- | :------ |
   | K4A_FRAMES_PER_SECOND_5 = 0 | 0: FPS=5. |
   | K4A_FRAMES_PER_SECOND_15 | 1: FPS=15. |
   | K4A_FRAMES_PER_SECOND_30 | 2: FPS=30. |

   ---

## Dependencies 

   OpenCV
