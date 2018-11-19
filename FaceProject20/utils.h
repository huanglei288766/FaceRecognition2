#pragma once

#define APPID (MPChar)"AsmmgSYCjbzNQBBdpGoEUbeabLQdsduiHHfmhtx3Xr27"
#define SDKKey (MPChar)"AXgb1kGgZbfm53KgVhMKzg9ca4dGcAYY2kH4qHurTwT2"

#pragma comment(lib, "lib/libarcsoft_face_engine.lib")

#define SafeFree(p) { if ((p)) free(p); (p) = NULL; }
#define SafeArrayDelete(p) { if ((p)) delete [] (p); (p) = NULL; } 
#define SafeDelete(p) { if ((p)) delete (p); (p) = NULL; } 