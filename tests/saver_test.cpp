#include "../firmware/saver.h"
#include <cassert>
#include <iostream>
int main(){
 Saver s;s.tick(0,true,0,true);assert(!s.on);s.tick(100,true,0,true);assert(s.on);
 s.tick(30099,true,40,false);assert(s.on);s.tick(30100,true,40,false);assert(!s.on);
 s.tick(30200,true,40,true);s.tick(30300,true,40,true);assert(s.on);s.tick(30400,true,500,false);assert(!s.on&&s.latched);
 assert(!s.reset(false));assert(s.reset(true));s.tick(31000,false,0,false);assert(s.latched);
 Saver w;w.on=true;w.idleSince=0xfffffff0u;w.tick(30000,true,40,false);assert(!w.on);
 Saver busy;busy.on=true;busy.tick(40000,true,100,false);assert(busy.on);busy.stop();assert(!busy.on&&busy.latched);
 Saver nan;nan.tick(0,true,NAN,false);assert(nan.latched&&!nan.on);
 std::cout<<"sound confirmation, idle timeout, busy hold, overload/invalid latch, reset/stop and wrap passed\n";
}
