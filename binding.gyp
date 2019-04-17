{
	"targets":[
		{
			"target_name": "phoenixreader",
			"cflags!": [ "-fno-exceptions" ],
			"cflags_cc!": [ "-fno-exceptions" ],
			"sources" : [ "cpp/wifi.cpp","cpp/inventory.cpp" ],
			"include_dirs" : [
				"<!@(node -p \"require('node-addon-api').include\")"
			],
			"libraries": [
				"-luhf","-llu9000","-llinuxhal","-lpthread"],
			'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
		}
	]
}
