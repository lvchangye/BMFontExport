# BMFontExport for Win
Make Bitmap png&amp;fnt from folder.
The output file name is same as INPUT_FOLDER name with png/fnt suffix.
Every png's name last character(without extension) will be used for unicode code

# Usage: BMFont.exe [-w 2048] INPUT_FOLDER

# Dependency
Opencv: Install it from https://opencv.org/releases/, put bin path to your environment PATH
Boost: Download from https://boostorg.jfrog.io/artifactory/main/release/,put boost root path to your environment with BOOST_ROOT.

# Test
You can test with "BMFont.exe testfolder",
Then the testfolder.fnt and testfolder.png will be exported.

