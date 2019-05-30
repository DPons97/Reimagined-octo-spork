# [PII]  Reimagined-Octo-Spork
#### Eterogeneous distributed system for object tracking and fun stuff

This project involves object recognition with Darknet and OpenCV's YOLO algorithm running on a distributed system for object tracking.

Requirements:
---

**Server-side**:
* @pjreddie's **Darknet YOLO** (Already included in this repo) - https://github.com/pjreddie/darknet/
  + ***.cfg** that are in this repository have been tweaked to be faster with less powerful NVIDIA GPUs*
  + ***yolov3.weights** + **yolov3-tiny.weights** files are required and need to be placed inside **Server/darknet/***

**Client-side:**
* **OpenCV 3.4.5** - https://github.com/opencv/opencv/releases/tag/3.4.5 
  + **OpenCV_DNN** additional modules - https://github.com/opencv/opencv_contrib/releases/tag/3.4.5
  + Installation guide here: https://docs.opencv.org/3.2.0/de/d25/tutorial_dnn_build.html

* A *LOT* of patience

Installation:
---
After cloning this repository (```git clone https://github.com/DPons97/reimagined-octo-spork.git ```):
* Build darknet inside ```Server/darknet/``` (and test it!)
* Build OctoSpork: 
  ```bash
  cd [...]/reimagined-octo-spork/
  mkdir build
  cmake ..
  make [leave empty if you want to build everything. Otherwise, see below]  
  ```
  If you want to only build Server, change ```make``` line to:
  ```shell
  make OctoSporkServer
  ```
  Otherwise, to build Client only:
  ```shell
  make OctoSporkClient
  ```
  
* If you decided to build Client, you have to move *build/bkgSubtraction* and *build/nodeTracker* inside ***Client/Executables***.


<br><br>
##### Tested Hardware:
* Odroid xu4
* Jetson-TX2 with Nvidia Tegra technology
* Any PC with any Linux distribution (Tested on Fedora and Ubuntu)
<br><br>
##### Project Contributors:
* Collini Luca (@Lucaz97) - luca.collini@mail.polimi.it
* Pons Davide (@DPons97) - davide.pons@mail.polimi.it
<br>
