#include <thread>
#include <iostream>
#include <map>
#include <napi.h>
#include <mutex>
#include "LU9000.h"
#include "inventory.h"

using namespace std;
using namespace LU9000;

ModuleAPI module;
bool antennaEnable[2];
int antennaPower[2];
int uhfMaxPower = 0;
thread *th;

struct tag_data
{
	int antenna;
	float rssi;
	int count;
};

Napi::Value open(const Napi::CallbackInfo &info)
{
	//cout << "open call" << endl;
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
	Type ret = module.Open(device.c_str(), 115200);
	if (ret == 0)
	{

		module.RecallModuleParam(&uhfband, &uhfMaxPower);
		int dwell = 2000;
		module.GetAntennaState(0, antennaEnable, &dwell, antennaPower);
		module.GetAntennaState(1, antennaEnable + 1, &dwell, antennaPower + 1);
		reader_status = OPEN;
	}
	else
	{
		module.Close();
	}
	return Napi::Number::New(env, ret);
}

Napi::Value close(const Napi::CallbackInfo &info)
{
	//cout << "close call" << endl;
	Napi::Env env = info.Env();
	Type ret;
	switch (reader_status)
	{

	case INVENTORY:
		ret = module.StopOperation();
		ret = module.Close();
		break;
	case OPEN:
		ret = module.Close();
		break;
	case IDLE:
	default:
		ret = OK;
		break;
	}
	reader_status = IDLE;
	return Napi::Number::New(env, ret);
}

Napi::Value stopOperation(const Napi::CallbackInfo &info)
{
	//cout << "stopOperation call" << endl;
	Napi::Env env = info.Env();
	if (reader_status == IDLE)
		return Napi::Number::New(env, -10);
	Type ret = module.StopOperation();
	return Napi::Number::New(env, ret);
}

Napi::Value getMaxPower(const Napi::CallbackInfo &info)
{
	//cout << "getMaxPower call" << endl;
	Napi::Env env = info.Env();

	if (uhfMaxPower != 0)
	{
		return Napi::Number::New(env, uhfMaxPower);
	}
	return Napi::Number::New(env, 0);
}

Napi::Value getAntennaPower(const Napi::CallbackInfo &info)
{
	//cout << "getAntennaPower call" << endl;
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
		return Napi::Number::New(env, antennaPower[antennaNo]);
	}
	else
	{
		Napi::TypeError::New(env, "Wrong antenna No").ThrowAsJavaScriptException();
		return env.Null();
	}
}

Napi::Value getAntennaState(const Napi::CallbackInfo &info)
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
		return Napi::Boolean::New(env, antennaEnable[antennaNo]);
	}
	else
	{
		Napi::TypeError::New(env, "Wrong antenna No").ThrowAsJavaScriptException();
		return env.Null();
	}
}

Napi::Value setFastMode(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	if (info.Length() != 1)
	{
		Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
		return env.Null();
	}
	if (!info[0].IsBoolean())
	{
		Napi::TypeError::New(env, "Wroing argument").ThrowAsJavaScriptException();
		return env.Null();
	}
	bool fastmode = info[0].As<Napi::Boolean>().Value();
	Type ret;
	if (fastmode)
	{
		cout << "fastmode " << endl;
		ret = module.SetLinkProfile(3);
		if (ret == OK)
		{
			cout << "setTxTime " << endl;
			ret = module.SetTxTime(400, 0);
		}
		if (ret == OK)
		{
			cout << "setQuernParam" << endl;
			ret = module.SetQueryParam(0, 2, 2, 4);
		}
	}
	else
	{
		ret = module.SetTxTime(200, 200);
		if (ret == OK)
		{
			ret = module.SetLinkProfile(1);
		}
		if (ret == OK)
		{
			ret = module.SetQueryParam(0, 2, 2, 4);
		}
	}
	return Napi::Number::New(env, ret);
}

Napi::Value setFrequency920(const Napi::CallbackInfo &info)
{
	Napi::Env env = info.Env();
	if (info.Length() != 1)
	{
		Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
		return env.Null();
	}
	if (!info[0].IsBoolean())
	{
		Napi::TypeError::New(env, "Wroing argument").ThrowAsJavaScriptException();
		return env.Null();
	}
	bool frequency920 = info[0].As<Napi::Boolean>().Value();
	Type ret = OK;
	unsigned int freq = frequency920 ? 0 : 1;
	cout << "frequency flag = " << freq << endl;
	module.WriteRegister(0x0C01, 0);
	module.WriteRegister(0x0C02, freq);
	module.WriteRegister(0x0C01, 6);
	module.WriteRegister(0x0C02, freq);
	module.WriteRegister(0x0C01, 12);
	module.WriteRegister(0x0C02, freq);
	module.WriteRegister(0x0C01, 31);
	module.WriteRegister(0x0C02, freq);
	module.WriteRegister(0x0C01, 32);
	module.WriteRegister(0x0C02, freq);
	module.WriteRegister(0x0C01, 33);
	module.WriteRegister(0x0C02, freq);
	return Napi::Number::New(env, ret);
}

Napi::Value setAntennaState(const Napi::CallbackInfo &info)
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
		Type ret = module.SetAntennaState(antennaNo, enable, 2000, power);

		return Napi::Number::New(env, ret);
	}
	else
	{
		Napi::TypeError::New(env, "Wrong antenna No").ThrowAsJavaScriptException();
		return env.Null();
	}
}

Napi::Value inventory(const Napi::CallbackInfo &info)
{
	//cout << "inventory call" << endl;
	Napi::Env env = info.Env();
	ModuleAPI::INVENTORY_CALLBACK inventory_function = [&](string &epccode, float rssi, float phase, int channel, int antenna) {
		////cout << "read epccode = " << epccode << endl;
	};

	th = new thread([&] {
		//cout << "inventory start" << endl;
		module.Inventory(false, inventory_function);
		//cout << "inventory end" << endl;
	});
	return Napi::Number::New(env, 0);
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
	//cout << "init call" << endl;
	exports.Set(Napi::String::New(env, "open"),
				Napi::Function::New(env, open));
	exports.Set(Napi::String::New(env, "close"),
				Napi::Function::New(env, close));
	exports.Set(Napi::String::New(env, "stop"),
				Napi::Function::New(env, stopOperation));
	exports.Set(Napi::String::New(env, "getmaxpower"),
				Napi::Function::New(env, getMaxPower));
	exports.Set(Napi::String::New(env, "getantennapower"),
				Napi::Function::New(env, getAntennaPower));
	exports.Set(Napi::String::New(env, "getantennastate"),
				Napi::Function::New(env, getAntennaState));
	exports.Set(Napi::String::New(env, "setantennastate"),
				Napi::Function::New(env, setAntennaState));
	exports.Set(Napi::String::New(env, "inventory"),
				Napi::Function::New(env, inventory));
	exports.Set(Napi::String::New(env, "setfastmode"),
				Napi::Function::New(env, setFastMode));
	exports.Set(Napi::String::New(env, "setfrequency920"),
				Napi::Function::New(env, setFrequency920));
	init_inventory(env, exports);
	return exports;
}

NODE_API_MODULE(phoenixreader, Init)
