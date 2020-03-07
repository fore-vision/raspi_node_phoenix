{
	"targets":[
		{
			"target_name": "phoenixreader",
			"cflags!": [ "-fno-exceptions" ],
			"cflags_cc!": [ "-fno-exceptions" ,'-std=c++14'],
			"sources" : [ "cpp/Rfid.cpp" ],
			"include_dirs" : [
				"<!@(node -p \"require('node-addon-api').include\")"
			],
			"libraries": [
				"-luhf","-llu9000","-lpthread"],
			'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
		}
	]
}
