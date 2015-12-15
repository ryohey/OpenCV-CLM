# OpenCV-CLM

Mac OSX port of the Constrained Local Model (CLM) OpenCV implementation 

Original: [Xiaoguang Yan's Webpage](https://sites.google.com/site/xgyanhome/home/projects/clm-implementation)

# Usage

## Install OpenCV

```
brew tap homebrew/science
brew install opencv
```

## Download Images

Place `all-images` directory that contains `franck_00000.jpg` ~ `franck_01999.jpg`

[http://www-prima.inrialpes.fr/FGnet/data/01-TalkingFace/talking_face.html](http://www-prima.inrialpes.fr/FGnet/data/01-TalkingFace/talking_face.html)

- First 1000 images
- Second 1000 images

```cpp:OpenCV_CLM.cpp
auto imagesDir = "/Users/ryohei/Desktop/all-images"; // fix this
```
