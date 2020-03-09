#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <thread>
#include "Rfid.h"

struct InventoryContext
{
  InventoryContext(Napi::Env env) // : deferred(Napi::Promise::Deferred::New(env))

  {
  }
  // Napi::Promise::Deferred deferred;
  std::thread inventoryThread;
  Napi::ThreadSafeFunction invfn;

  Rfid *pRfid;
};

void inventory(InventoryContext *context);
void FinalizerCallback(Napi::Env env, void *finalizeDta, InventoryContext *context);

Napi::Value Rfid::Inventory(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  auto inventoryContext = new InventoryContext(env);
  inventoryContext->pRfid = this;
  inventoryContext->invfn = Napi::ThreadSafeFunction::New(env,
                                                          info[0].As<Napi::Function>(),
                                                          "Inventory",
                                                          0,
                                                          1,
                                                          inventoryContext,
                                                          FinalizerCallback, inventoryContext);
  inventoryContext->inventoryThread = std::thread(inventory, inventoryContext);

  return Napi::Boolean::New(env, true);
}

struct EpcData
{
  std::string epccode;
  float rssi;
  float phase;
  int channel;
  int antenna;
};

void inventory(InventoryContext *context)
{
  auto func = [](std::string &epccode, float rssi, float phase, int channel, int antenna, void *data) -> void {
    auto callbackJS = [](Napi::Env env, Napi::Function jsCallback, void *epc) {
      struct EpcData *epcData = (struct EpcData *)epc;
      Napi::Object obj = Napi::Object::New(env);
      obj.Set("epccode", epcData->epccode);
      obj.Set("rssi", epcData->rssi);
      obj.Set("phase", epcData->phase);
      obj.Set("channel", epcData->channel);
      obj.Set("antenna", epcData->antenna);

      jsCallback.Call({obj});
      delete epcData;
    };
    struct EpcData *epcData = new EpcData();
    epcData->epccode = epccode;
    epcData->rssi = rssi;
    epcData->phase = phase;
    epcData->channel = channel;
    epcData->antenna = antenna;

    InventoryContext *inv = (InventoryContext *)data;
    inv->invfn.BlockingCall(epcData, callbackJS);
  };
  context->pRfid->reader_status = INVENTORY;
  context->pRfid->inventoryThread(context, func);
  context->pRfid->reader_status = OPEN;
  context->invfn.Release();
  //delete context;
}

void FinalizerCallback(Napi::Env env, void *finalizeData, InventoryContext *context)
{
  context->inventoryThread.join();

  //  context->deferred.Resolve(Napi::Boolean::New(env, true));
  delete context;
}

Napi::Value Rfid::Stop(const Napi::CallbackInfo &Info)
{
  Napi::Env env = Info.Env();
  if (reader_status == IDLE)
    return Napi::Number::New(env, -10);
  Type ret = moduleApi.StopOperation();
  return Napi::Number::New(env, ret);
}
