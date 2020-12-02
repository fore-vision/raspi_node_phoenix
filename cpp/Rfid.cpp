#include "Rfid.h"

Napi::FunctionReference Rfid::s_constructor;

Napi::Object Rfid::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func =
        DefineClass(env, "rfid", {InstanceMethod("Open", &Rfid::Open), InstanceMethod("Close", &Rfid::Close), InstanceMethod("GetAntennaState", &Rfid::GetAntennaState), InstanceMethod("SetAntennaState", &Rfid::SetAntennaState), InstanceMethod("SetQueryParam", &Rfid::SetQueryParam), InstanceMethod("GetQueryParam", &Rfid::GetQueryParam), InstanceMethod("GetMaxPower", &Rfid::GetMaxPower), InstanceMethod("Inventory", &Rfid::Inventory), InstanceMethod("Stop", &Rfid::Stop)});

    s_constructor = Napi::Persistent(func);
    s_constructor.SuppressDestruct();
    exports.Set("rfid", func);
    return exports;
}
Rfid::Rfid(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Rfid>(info)
{
    Napi::Env env = info.Env();
    reader_status = IDLE;
    uhfMaxPower = 0;
}

Napi::Value Rfid::Open(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    int uhfband = 0;

    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsString())
    {
        Napi::TypeError::New(env, "Wrong argument").ThrowAsJavaScriptException();
        return env.Null();
    }
    if (reader_status != IDLE)
        return Napi::Number::New(env, -10);
    string device = info[0].As<Napi::String>().Utf8Value();
    Type ret = moduleApi.Open(device.c_str(), 115200);
    if (ret == 0)
    {

        moduleApi.RecallModuleParam(&uhfband, &uhfMaxPower);
        reader_status = OPEN;
    }
    else
    {
        moduleApi.Close();
    }
    return Napi::Number::New(env, ret);
}

Napi::Value Rfid::Close(const Napi::CallbackInfo &info)
{
    //cout << "close call" << endl;
    Napi::Env env = info.Env();
    Type ret;
    switch (reader_status)
    {

    case INVENTORY:
        ret = moduleApi.StopOperation();
        moduleApi.Close();
        break;
    case OPEN:
        moduleApi.Close();
        ret = OK;
        break;
    case IDLE:
        ret = OK;
        break;
    default:
        ret = OK;
        break;
    }
    reader_status = IDLE;
    return Napi::Number::New(env, ret);
}

Napi::Value Rfid::GetMaxPower(const Napi::CallbackInfo &info)
{
    //cout << "getMaxPower call" << endl;
    Napi::Env env = info.Env();

    if (uhfMaxPower != 0)
    {
        return Napi::Number::New(env, uhfMaxPower);
    }
    return Napi::Number::New(env, 0);
}

Napi::Value Rfid::GetAntennaState(const Napi::CallbackInfo &info)
{
    //cout << "getAntennaState call" << endl;
    Napi::Env env = info.Env();
    if (info.Length() != 1)
    {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    if (!info[0].IsNumber())
    {
        Napi::TypeError::New(env, "Wrong argument").ThrowAsJavaScriptException();
        return env.Null();
    }
    int antennaNo = info[0].As<Napi::Number>().Int64Value();
    if (antennaNo == 0 || antennaNo == 1)
    {
        bool enable;
        int dwell;
        int power;
        Type ret = moduleApi.GetAntennaState(antennaNo, &enable, &dwell, &power);
        if (ret == 0)
        {
            Napi::Object obj = Napi::Object::New(env);
            obj.Set("enable", enable);
            //obj.Set("dwell", dwell);
            obj.Set("power", power);
            return obj;
        }
        else
        {
            Napi::TypeError::New(env, "Internal error").ThrowAsJavaScriptException();
            return env.Null();
        }
    }
    else
    {
        Napi::TypeError::New(env, "Wrong antenna No").ThrowAsJavaScriptException();
        return env.Null();
    }
}

Napi::Value Rfid::SetAntennaState(const Napi::CallbackInfo &info)
{
    //cout << "setAntennaState call" << endl;
    Napi::Env env = info.Env();
    if (info.Length() != 3)
    {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    if (!info[0].IsNumber())
    {
        Napi::TypeError::New(env, "Wrong argument").ThrowAsJavaScriptException();
        return env.Null();
    }
    int antennaNo = info[0].As<Napi::Number>().Int64Value();
    if (!info[1].IsBoolean())
    {
        Napi::TypeError::New(env, "Wrong argument").ThrowAsJavaScriptException();
        return env.Null();
    }
    bool enable = info[1].As<Napi::Boolean>().Value();

    if (!info[2].IsNumber())
    {
        Napi::TypeError::New(env, "Wrong argument").ThrowAsJavaScriptException();
        return env.Null();
    }
    int power = info[2].As<Napi::Number>().Int64Value();
    if (power < 0 || power > uhfMaxPower)
    {
        Napi::TypeError::New(env, "Wrong power valuse").ThrowAsJavaScriptException();
        return env.Null();
    }
    //cout << "antenna = " << antennaNo << " enable = " << enable << " power = " << power << endl;
    if (reader_status != OPEN)
        return Napi::Number::New(env, -10);
    if (antennaNo == 0 || antennaNo == 1)
    {
        Type ret = moduleApi.SetAntennaState(antennaNo, enable, 2000, power);

        return Napi::Number::New(env, ret);
    }
    else
    {
        Napi::TypeError::New(env, "Wrong antenna No").ThrowAsJavaScriptException();
        return env.Null();
    }
}

Napi::Value Rfid::SetQueryParam(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() != 3)
    {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    if (!info[0].IsNumber())
    {
        Napi::TypeError::New(env, "Wrong argument").ThrowAsJavaScriptException();
        return env.Null();
    }
    int sel = info[0].As<Napi::Number>().Int64Value();
    if (!info[1].IsNumber())
    {
        Napi::TypeError::New(env, "Wrong argument").ThrowAsJavaScriptException();
        return env.Null();
    }
    int session = info[1].As<Napi::Number>().Int64Value();
    if (!info[2].IsNumber())
    {
        Napi::TypeError::New(env, "Wrong argument").ThrowAsJavaScriptException();
        return env.Null();
    }
    int target = info[2].As<Napi::Number>().Int64Value();
    if (!info[3].IsNumber())
    {
        Napi::TypeError::New(env, "Wrong argument").ThrowAsJavaScriptException();
        return env.Null();
    }
    int q = info[3].As<Napi::Number>().Int64Value();
    Type type = moduleApi.SetQueryParam(sel, session, target, q);
    return Napi::Number::New(env, type);
}
Napi::Value Rfid::GetQueryParam(const Napi::CallbackInfo &info)
{
    int sel, session, target, q;
    Napi::Env env = info.Env();
    Type type = moduleApi.GetQueryParam(&sel, &session, &target, &q);
    if (type == 0)
    {
        Napi::Object obj = Napi::Object::New(env);
        obj.Set("sel", sel);
        obj.Set("session", session);
        obj.Set("target", target);
        obj.Set("q", q);
        return obj;
    }
    else
    {
        Napi::TypeError::New(env, "Internal error").ThrowAsJavaScriptException();
        return env.Null();
    }
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    Rfid::Init(env, exports);
    return exports;
}

NODE_API_MODULE(phoenixreader, Init)