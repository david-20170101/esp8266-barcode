--app.lua
local module = {}

local mqtt_connected = 0

function module.serial_number()
    print("[[$MQTT," .. config.CMD .. "," .. config.ID .. ",]]")
end

function module.mqtt_upload(upload)
    ok, table = pcall(sjson.decode, upload)
    if ok then
       if table.CardID and table.SN==config.ID and table.Reference then
          table.SN = nil
          table.MAC = config.ID
          module.mqtt_start(table)
       end
    end
end

local function mqtt_modf(nn)
    local m1 = 0
    local m2 = nn/256
    while m2>=1 do
       m1 = m1 + 1
       m2 = m2 - 1
    end
    m2 = m2*256
    return m1, m2
end

local function mqtt_public(con, temp, humi, status, upload)
    local mqtt = {}
    ok, s = pcall(sjson.encode, upload)
    if ok then
       SimpleJsonWithoutTime = s
       ok, t1, t2 = pcall(mqtt_modf, string.len(SimpleJsonWithoutTime))
       if ok then
          local bytes = string.char(3) .. string.char(t1) .. string.char(t2) .. SimpleJsonWithoutTime
          con:publish(config.uploadPOINT, bytes, 1, 0, function(conn) 
              print("Successfully Published to MQTT broker ")
              mqtt_connected=1 end,
              function(client, reason) 
                  print("Disconnected from ......") 
                  print("failed reason: " .. reason)
                  mqtt_connected = 0
              end)
       end
	else
       if status == dht.OK or status == dht.ERROR_CHECKSUM then
          mqtt[config.UID] = config.ID
          mqtt["Temperature"] = temp
          mqtt["Humidity"] = humi
          mqtt[config.USMART] = config.SMART
          mqtt[config.USWITCH] = config.SWITCH
       else
          mqtt[config.UID] = config.ID
          mqtt[config.USMART] = config.SMART
          mqtt[config.USWITCH] = config.SWITCH
       end
       SimpleJsonWithoutTime = sjson.encode(mqtt)
       ok, t1, t2 = pcall(mqtt_modf, string.len(SimpleJsonWithoutTime))
       if ok then
          local bytes = string.char(3) .. string.char(t1) .. string.char(t2) .. SimpleJsonWithoutTime
          con:publish(config.ENDPOINT, bytes, 1, 0, function(conn) 
              print("Successfully Published to MQTT broker ")
              mqtt_connected=1 end,
              function(client, reason) 
                  print("Disconnected from ......") 
                  print("failed reason: " .. reason)
                  mqtt_connected = 0
              end)
       end
    end
end

function module.mqtt_start(upload)
    local status, temp, humi, temp_dec, humi_dec = dht.read11(config.DHT)
    if status == dht.OK then
        print("DHT Temperature:"..temp..";".."Humidity:"..humi)
    elseif status == dht.ERROR_CHECKSUM then
        print( "DHT Checksum error." )
    elseif status == dht.ERROR_TIMEOUT then
        print( "DHT timed out." )
    end
    if mqtt_connected == 0 then
        -- initiate the mqtt client and set keepalive timer to 120sec
        m = mqtt.Client(config.CLIENTID, config.KEEPALIVE, config.USERNAME, config.PASSWORD, true, config.MAX_MESSAGE_LENGTH)
        -- register connect offline message overflow callback beforehand
        m:on("connect", function(con) print ("mqtt:on connected") mqtt_connected=1 end)
        m:on("offline", function(con) print ("mqtt:on offline") mqtt_connected=0 end)
        m:on("message", function(conn, topic, data)
            if data ~= nil then
                ok, t = pcall(sjson.decode, data)
                if ok then
                    config.json = t
                    if ( config.json.id == "SMART" ) then
                        config.SmartConfig("config.lua", "config.tmp", "module.SMART", config.json.value)
                        node.restart()                        
                    end
                    if ( config.json.id == "REBOOT" and config.json.value == "1" ) then
                        node.restart()
                    end
                    --'{"id":"CONFIG","value":[{"id":"CLIENTID","value":"517142935"},{"id":"USERNAME","value":"197087"},{"id":"PASSWORD","value":"4NUYcwFUq58IFKkQgFIWElbNWZk="}]}'
                    if ( config.json.id == "CONFIG" ) then
                        for key, table in pairs(config.json.value) do 
                            config.SmartConfig("config.lua", "config.tmp", "module." .. table.id , table.value)
	                    end
                    end
                    if ( config.json.id == "SWITCH" ) then 
                         if ( config.json.value == "1" ) then
                              gpio.write(config.PINSWITCH, gpio.HIGH)
                         elseif ( config.json.value == "0" ) then
                              gpio.write(config.PINSWITCH, gpio.LOW)
                         end
                         --config.SmartConfig("config.lua", "config.tmp", "module.SWITCH", config.json.value)
                         config.SWITCH = config.json.value
                         mqtt_public(conn)
                    end
                else
                    if (string.sub(data,1,1)==string.char(2)) then
                        local tmp = string.sub(data,4)
                        ok, table = pcall(sjson.decode, tmp)
                        if ok then
                           if (table.MAC==config.ID and table.Reference and table.CardID and table.BarCode and table.StyleNo and table.ColorNo and table.SizeNo and table.StyleName and table.ColorName and table.SizeName) then
                               print("[[$MQTT," .. config.CMD .. "," .. config.ID .. "," .. table.Reference .. "," .. table.CardID .. "," .. table.BarCode .. "," .. table.StyleNo .. "," .. table.ColorNo .. "," .. table.SizeNo .. "," .. table.StyleName .. "," .. table.ColorName .. "," .. table.SizeName .. ",]]" )
                           elseif (table.MAC==config.ID and table.Reference) then
                               print("[[$MQTT," .. config.CMD .. "," .. config.ID .. "," .. table.Reference .. ",]]" )
                           end
                        end
                    end
                end
            end
        end)
        m:on("overflow", function(client, topic, data)
            print(topic .. " partial overflowed message: " .. data )
            mqtt_connected = 0
        end)

        -- Connect to broker -- Connect to broker
        m:connect(config.HOST, config.PORT, 0, 0, function(conn) 
            print("Successfully Connected to MQTT broker ") 
            mqtt_connected = 1 
            -- subscribe topic with qos = 1
            m:subscribe(config.responsePOINT, 1, function(conn) 
                 print("Successfully Subscribe to MQTT broker ") 
            end)
            -- Publish -- Publish -- Publish -- Publish
            mqtt_public(m, temp, humi, status, upload)
        end,
        function(client, reason) 
            print("Disconnected from ......") 
            print("failed reason: " .. reason)
            mqtt_connected = 0
        end)
    elseif (mqtt_connected == 1) then
            mqtt_public(m, temp, humi, status, upload)
    end
end

function module.start()
    gpio.mode(config.PINSWITCH, gpio.OUTPUT)
    if (config.SWITCH=="1") then
        gpio.write(config.PINSWITCH, gpio.HIGH)
    else
        gpio.write(config.PINSWITCH, gpio.LOW)
    end
    local my = tmr.create()
    tmr.alarm(my, config.INTERVAL, tmr.ALARM_AUTO, app.mqtt_start)
end

return module
