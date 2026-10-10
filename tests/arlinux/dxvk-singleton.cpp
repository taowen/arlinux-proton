#include <cstdio>
#include <stdexcept>
#include "util/rc/util_rc.h"
#include "util/util_singleton.h"
struct Object: dxvk::RcObject {
  static int attempts, destroyed;
  Object(bool fail) { ++attempts; if(fail)throw std::runtime_error("unsupported adapter"); }
  ~Object() { ++destroyed; }
};
int Object::attempts=0;
int Object::destroyed=0;
int main() {
  FILE *f=fopen("C:\\singleton-probe.txt","w"); if(!f)return 99;
  dxvk::Singleton<Object> singleton;
  for(int i=0;i<3;++i) {
    try {
      auto p=singleton.acquire(true);
      fprintf(f,"FAIL: acquire returned %s after failed construction\n",p.ptr()?"an object":"null");
      fclose(f); return 1;
    }catch(const std::runtime_error&) { fprintf(f,"expected failure %d\n",i+1); }
  }
  auto a=singleton.acquire(false), b=singleton.acquire(false);
  if(!a.ptr()||a.ptr()!=b.ptr()||Object::attempts!=4) {fputs("FAIL: successful acquire\n",f); fclose(f);return 2;}
  singleton.release();singleton.release();a=nullptr;b=nullptr;
  auto c=singleton.acquire(false);singleton.release();c=nullptr;
  bool ok=Object::attempts==5&&Object::destroyed==2;
  fprintf(f,"%s: attempts=%d destroyed=%d\n",ok?"PASS":"FAIL",Object::attempts,Object::destroyed);
  fclose(f);return ok?0:3;
}
