var cachedData = {
	"get": {},
	"clear": function(dataArray){$.each(dataArray, function(idx, val){delete cachedData.get[val];})}
}

var asyncData = {
	"get": {},
	"clear": function(dataArray){$.each(dataArray, function(idx, val){delete asyncData.get[val];})}
}

var httpApi ={
	"nvramGet": function(objItems, forceUpdate){
		var queryArray = [];
		var retData = {};

		var __nvramget = function(_nvrams){
			return _nvrams.map(function(elem){return "nvram_get(" + elem + ")";}).join("%3B");
		};

		if(forceUpdate) cachedData.clear(objItems);

		objItems.forEach(function(key){
			if(cachedData.get.hasOwnProperty(key)){
				retData[key] = cachedData.get[key];
			}
			else if(asyncData.get.hasOwnProperty(key)){
				retData[key] = cachedData.get[key] = asyncData.get[key];
				if(forceUpdate) delete asyncData.get[key];
			}
			else{
				queryArray.push(key);
			}
		});

		if(queryArray.length != 0){
			$.ajax({
				url: '/appGet.cgi?hook=' + __nvramget(queryArray),
				dataType: 'json',
				async: false,
				error: function(){
					for(var i=0; i<queryArray.length; i++){retData[queryArray[i]] = "";}
					retData.isError = true;

					$.ajax({
						url: '/appGet.cgi?hook=' + __nvramget(queryArray),
						dataType: 'json',
						error: function(){
							for(var i=0; i<queryArray.length; i++){asyncData.get[queryArray[i]] = "";}
						},
						success: function(response){
							Object.keys(response).forEach(function(key){asyncData.get[key] = response[key];})
						}
					});
				},
				success: function(response){
					Object.keys(response).forEach(function(key){retData[key] = cachedData.get[key] = response[key];})
					retData.isError = false;
				}
			});
		}
		else{
			retData.isError = false;		
		}
		
		return retData;
	},

	"nvramSet": function(postData, handler){
		delete postData.isError;

		$.ajax({
			url: '/applyapp.cgi',
			dataType: 'json',
			data: postData,
			error: function(response){
				if(handler) handler.call(response);
			},
			success: function(response){
				if(handler) handler.call(response);
			}
		})
	},

	"hookGet": function(hookName, forceUpdate){
		var queryString = hookName.split("-")[0] + "(" + (hookName.split("-")[1] || "") + ")";
		var retData = {};

		if(cachedData.get.hasOwnProperty(hookName) && !forceUpdate){
			retData[hookName] = cachedData.get[hookName];
		}
		else if(asyncData.get.hasOwnProperty(hookName)){
			retData[hookName] = asyncData.get[hookName];
			if(forceUpdate) delete asyncData.get[hookName];
		}
		else{
			$.ajax({
				url: '/appGet.cgi?hook=' + queryString,
				dataType: 'json',
				async: false,
				error: function(){
					retData[hookName] = "";
					retData.isError = true;
			
					$.ajax({
						url: '/appGet.cgi?hook=' + queryString,
						dataType: 'json',
						error: function(){
							asyncData.get[hookName] = "";
						},
						success: function(response){
							asyncData.get[hookName] = response[hookName];
						}
					});
				},
				success: function(response){
					retData = response;
					cachedData.get[hookName] = response[hookName]
					retData.isError = false;
				}
			});
		}

		return retData;
	},

	"startAutoDet": function(){
		$.get("/appGet.cgi?hook=start_autodet()");
	},

	"detwanGetRet": function(){
		var retData = {
			"wanType": "UNKNOWN",
			"isIPConflict": false,
			"isError": false
		};

		var getDetWanStatus = function(state){
			switch(state){
				case 0:
					if(hadPlugged("modem"))
						retData.wanType = "MODEM";
					else
						retData.wanType = "NOWAN";
					break;
				case 2:
				case 5:
					retData.wanType = "DHCP";
					break;
				case 3:
				case 6:
					retData.wanType = "PPPoE";
					break;
				case 4:
				case "":
					retData.wanType = "CHECKING";
					break;
				case 7:
					retData.wanType = "DHCP";
					retData.isIPConflict = true;
					break;
				default:
					if(hadPlugged("modem")){
						retData.wanType = "MODEM";
					}
					else{
						retData.wanType = "UNKNOWN";
					}
					break;
			}
		}

		$.ajax({
			url: '/detwan.cgi?action_mode=GetWanStatus',
			dataType: 'json',
			async: false,
			error: function(xhr){
				if(asyncData.get.hasOwnProperty("detwanState")){
					getDetWanStatus(asyncData.get["detwanState"]);
					retData.isError = false;

					delete asyncData.get["detwanState"];
				}
				else{
					retData = {
						"wanType": "CHECKING",
						"isIPConflict": false,
						"isError": true
					}

					$.ajax({
						url: '/detwan.cgi?action_mode=GetWanStatus',
						dataType: 'json',
						error: function(xhr){
							asyncData.get["detwanState"] = "UNKNOWN";
						},
						success: function(response){
							asyncData.get["detwanState"] = response.state;
						}
					});
				}
			},
			success: function(response){
				getDetWanStatus(response.state)
			}
		});

		return retData;
	},

	"isAlive": function(){
		var retData = false;

		if(asyncData.get.hasOwnProperty("isAlive")){
			retData = asyncData.get.isAlive;
			delete asyncData.get.isAlive;
		}

		$.ajax({ 
			url: "/repage.json",
			dataType: "script",
			timeout: 5000,
			error: function(){
				asyncData.get.isAlive = false;
			},
			success: function(){
				asyncData.get.isAlive = true;
			}
		});

		return retData;
	}
}