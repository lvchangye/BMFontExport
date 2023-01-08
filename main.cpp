#include "stdafx.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <codecvt>
#include <exception>
#include <fstream>
#include <io.h>
#include <iostream>
#include <locale>
#include <opencv2\opencv.hpp>
#include <string>
#include <vector>

using namespace std;
using namespace cv;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

#include <Windows.h>

wstring s2ws(const string str) {
  int len;
  int slength = (int)str.length();
  len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
  std::wstring r(len, L'\0');
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, &r[0], len);
  return r;
}

std::string ws2s(const std::wstring &s) {
  int len;
  int slength = (int)s.length();
  len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
  std::string r(len, '\0');
  WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
  return r;
}

cv::Mat ReadImage(const wchar_t *filename) {
  FILE *fp = nullptr;
  if (_wfopen_s(&fp, filename, L"rb")) {
    return Mat::zeros(1, 1, CV_8U);
  }
  fseek(fp, 0, SEEK_END);
  long sz = ftell(fp);
  char *buf = new char[sz];
  fseek(fp, 0, SEEK_SET);
  fread(buf, 1, sz, fp);
  _InputArray arr(buf, sz);
  Mat img = imdecode(arr, IMREAD_UNCHANGED);
  delete[] buf;
  fclose(fp);
  return img;
}

bool mergeDiffPic(const vector<wstring> &pic_list, int maxWidth,
                  const wstring &input_folder) {

  int pic_cols = 0;
  int pic_rows = 0;
  int max_cols = 0;
  int max_rows = 0;
  int size_cols = 0;
  int size_rows = 0;
  vector<int> tmp_cols;
  vector<int> tmp_rows;

  fs::path path(input_folder);
  wstring outout_fnt = path.wstring();
  outout_fnt.append(L".fnt");

  wstring png_file_name = path.filename().wstring();
  png_file_name.append(L".png");

  wstring out_png = path.parent_path().append(png_file_name).wstring();

  ofstream ofile(outout_fnt);
  if (!ofile.is_open()) {
    cout << "Unable to open file";
    return false;
  }

  size_t pic_num = pic_list.size();

  stringstream out_chars;
  out_chars << "chars count=" << to_string(pic_num) << "\n";
  int max_png_height = 0;
  int max_width = 0;
  int max_height = 0;

  vector<Mat> input(pic_num);
  vector<Mat> temp(pic_num);
  vector<Point> xy(pic_num);
  Mat merge;
  int fromx = 2, fromy = 2;
  for (int i = 0; i < pic_num; i++) {
    fs::path png_tmp(pic_list[i]);
    wstring s =
        png_tmp.stem().wstring().substr(png_tmp.stem().wstring().size() - 1, 1);

    input[i] = ReadImage(pic_list[i].c_str());
    int width = input[i].size().width;
    int height = input[i].size().height;
    if (fromx + width > maxWidth) {
      fromx = 2;
      fromy += max_png_height + 2;
    }

    xy[i] = Point(fromx, fromy);
    out_chars << "char id=" << to_string((int)s[0]);
    out_chars << " x=" << to_string(fromx);
    out_chars << " y=" << to_string(fromy);
    out_chars << " width=" << to_string(width);
    out_chars << " height=" << to_string(height);
    out_chars << " xoffset=0 yoffset=0 xadvance=" << to_string(width) << "\n";
    max_png_height = max(max_png_height, height);
    fromx += width + 2;
    max_width = max(max_width, fromx);
  }
  max_height = fromy + max_png_height;

  Size mergesize(max_width, max_height);
  merge.create(mergesize, CV_MAKETYPE(input[0].depth(), input[0].channels()));
  merge = Scalar::all(0);
  for (int i = 0; i < pic_num; i++) {
    input[i].copyTo(
        merge(Rect(xy[i].x, xy[i].y, input[i].cols, input[i].rows)));
  }

  stringstream out_content;
  out_content << "info face=\"楷体\" size=";
  out_content << to_string(max_png_height);
  out_content << " unicode=1 stretchH=100 smooth=1 aa=1 padding=0,0,0,0 "
                 "spacing=1,1 outline=0\n";
  out_content << "common lineHeight=";
  out_content << to_string(max_png_height);
  out_content << " base=";
  out_content << to_string(max_png_height);
  out_content << " scaleW=";
  out_content << to_string(maxWidth);
  out_content << " scaleH=";
  out_content << to_string(max_height);
  out_content << " pages=1 packed=0\n";
  out_content << "page id=0 file=\"" << ws2s(png_file_name) << "\"\n";

  if (ofile.is_open()) {
    ofile << out_content.str() << out_chars.str();
    ofile.close();
  }
  vector<int> compression_params;
  compression_params.push_back(IMWRITE_PNG_COMPRESSION);
  compression_params.push_back(6);
  imwrite(ws2s(out_png).c_str(), merge, compression_params);
  return true;
}

void getAllFiles(wstring path, vector<wstring> &files) {
  intptr_t hFile = 0;
  struct _wfinddata_t fileinfo;

  wchar_t *p;
  size_t psize = path.size() + 3;
  p = new wchar_t[psize];
  wcscpy_s(p, psize, path.c_str());
  wcscat_s(p, psize, L"/*");
  if ((hFile = _wfindfirst(p, &fileinfo)) != -1) {
    do {
      if (wcscmp(fileinfo.name, L".") != 0 &&
          wcscmp(fileinfo.name, L"..") != 0) {
        wstring str(path);
        str.append(L"/");
        str.append(fileinfo.name);
        files.push_back(str);
      }
    } while (_wfindnext(hFile, &fileinfo) == 0);

    _findclose(hFile);
  }
}

int main(int argc, char **argv) {
  po::options_description desc("All options");
  desc.add_options()(
      "help,h", "Make Bitmap png&fnt from INPUT_FOLDER.\nThe output file name "
                "is same as INPUT_FOLDER name with png/fnt suffix.\nEvery "
                "png's name last character(without extension) will be "
                "used for unicode code:\n\nUsage: BMFont [-w "
                "2048] INPUT_FOLDER\n")("maxwidth,w",
                                        po::value<int>()->default_value(2048),
                                        " the output png max width");
  po::options_description hidden("Hidden options");
  hidden.add_options()("input-folder", po::value<string>(), "input folder");

  po::positional_options_description p;
  p.add("input-folder", -1);

  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);

  po::variables_map vm;

  try {
    po::store(po::command_line_parser(argc, argv)
                  .options(cmdline_options)
                  .positional(p)
                  .run(),
              vm);
    po::notify(vm);
  } catch (exception &e) {
    cout << e.what() << endl;
    return 1;
  }

  if (vm.count("help")) {
    cout << desc << endl;
    return 0;
  }

  if (vm.count("input-folder") == 0) {
    cout << desc << endl;
    return 0;
  }

  if (vm.count("maxwidth") && vm.count("input-folder")) {
    int maxw = vm["maxwidth"].as<int>();
    cout << vm["input-folder"].as<string>() << endl;
    wstring inputFolder = s2ws(vm["input-folder"].as<string>());

    vector<wstring> pic_list;

    getAllFiles(inputFolder, pic_list);
    mergeDiffPic(pic_list, maxw, inputFolder);
  }

  return 0;
}