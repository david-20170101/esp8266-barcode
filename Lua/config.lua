-- config.lua
local module = {}

module.SMART = "0"
module.USMART = "SMART"
module.PINSWITCH = 3
module.SWITCH = "0"
module.USWITCH = "SWITCH"
module.DHT = 4
module.HOST = "183.230.40.39"
module.PORT = 6002
module.CLIENTID = "xxxxxxxxx"
module.USERNAME = "xxxxxx"
module.PASSWORD = "xxxxxxxxxxxxxxxxxxxxxxxxxxxx"
module.KEEPALIVE = 120
module.MAX_MESSAGE_LENGTH = 1024
module.ID = node.chipid() .. node.flashid()
module.UID = "MAC"
module.INTERVAL = 240000
module.ENDPOINT = "$dp"
module.uploadPOINT = "update"
module.responsePOINT = "response"
module.CardID = "CardID"
module.json = {}

function module.SmartConfig(oldfile, newfile, pattern, repl)
    local fd = file.open(oldfile, "r")
    local fw = file.open(newfile, "w")
    if type(repl) == "string" then repl = '"' .. repl .. '"' end
    if fd then
        result = fd:readline()
        while result ~= nil 
        do
            if string.find(result, pattern) then
               result = pattern .. " = " .. repl
            end
            if string.find(result, "\n") then
               result = string.gsub(result, "\n", "")
            end
            fw:writeline(result)
            result = fd:readline()
        end
    end
    fw:flush()
    fd:close()
    fw:close()
    local fd = nil
    local fw = nil
    file.remove(oldfile)
    file.rename(newfile, oldfile)
end 

return module