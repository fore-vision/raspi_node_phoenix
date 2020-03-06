#include <napi.h>

#include "LU9000.h"
using namespace LU9000;
enum
{
    IDLE = 0,
    OPEN,
    INVENTORY
};
class Rfid : public Napi::ObjectWrap<Rfid>
{
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Rfid(const Napi::CallbackInfo &info);

private:
    int reader_status;
    int uhfMaxPower;
    static Napi::FunctionReference constructor;
    ModuleAPI moduleApi;
    Napi::Value Open(const Napi::CallbackInfo &info);
    Napi::Value SetAntennaState(const Napi::CallbackInfo &info);
    Napi::Value GetMaxPower(const Napi::CallbackInfo &info);
    Napi::Value Close(const Napi::CallbackInfo &info);
    //Napi::Value Inventory(const Napi::CallbackInfo &info);
    //Napi::Value Stop(const Napi::CallbackInfo &Info);
};