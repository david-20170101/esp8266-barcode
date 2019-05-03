--setup.lua
local module = {}

local function start_smart()
    print("Smart Configuring Wifi ...")
    wifi.setmode(wifi.STATION)
    wifi.startsmart(1, function(ssid, password)
        print(string.format("Success. SSID:%s ; PASSWORD:%s", ssid, password))
        config.SmartConfig("config.lua", "config.tmp", "module.SMART", "0" )
        config.SMART = "0"
        wifi.stopsmart()
    end) 
end

-----[注册wifi事件]----
wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(T)
    print("\n====================================")
    print("ESP8266 mode is: " .. wifi.getmode())
    print("Chip ID "..node.chipid());
    print("MAC address is: " .. wifi.ap.getmac())
    print("IP is "..wifi.sta.getip())
    print("====================================")
    app.start()
end)

wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, function(T)
    print("Configuring Wifi Failure ......")
end)


function module.start()
    if ( config.SMART=="1" ) then
        start_smart()
    end
end

return module
