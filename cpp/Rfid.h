#include <napi.h>

#include "LU9000.h"

#ifndef __RFID_H__
#define __RFID_H__
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
    void inventoryThread(void *data, libuhf::INVENTORY_CALLBACK callback)
    {
        moduleApi._inventoryData = data;
        moduleApi.Inventory(true, callback);
    }
    int reader_status;

private:
    int uhfMaxPower;
    static Napi::FunctionReference s_constructor;
    ModuleAPI moduleApi;
    Napi::Value Open(const Napi::CallbackInfo &info);
    Napi::Value SetAntennaState(const Napi::CallbackInfo &info);
    Napi::Value GetAntennaState(const Napi::CallbackInfo &info);
    Napi::Value SetQueryParam(const Napi::CallbackInfo &info);
    Napi::Value GetQueryParam(const Napi::CallbackInfo &info);
    Napi::Value GetTxTime(const Napi::CallbackInfo &info);
    Napi::Value SetTxTime(const Napi::CallbackInfo &info);
    Napi::Value GetMaxPower(const Napi::CallbackInfo &info);
    Napi::Value Close(const Napi::CallbackInfo &info);
    Napi::Value Inventory(const Napi::CallbackInfo &info);
    Napi::Value Stop(const Napi::CallbackInfo &Info);
};

#endif