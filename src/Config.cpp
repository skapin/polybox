#include "Config.h"

Config::Config()
{
}

QString Config::pathToHomeDirectory="/home/poly/";
QString Config::pathToLinuxCNC=Config::pathToHomeDirectory+"linuxcnc/";
QString Config::pathToWebcamDevice="/dev/";
QString Config::webcamName="video0";
QString Config::linuxCNCCommand="linuxcnc";