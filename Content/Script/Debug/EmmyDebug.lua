--
-- EmmyLua debugger bootstrap for UnLua + IntelliJ IDEA.
-- Pair with run configuration "UnLua Emmy" (IDE listens, Lua connects).
-- Usage: start that debug config first, then PIE in Unreal.
--

local HOST = "127.0.0.1"
local PORT = 9966

local function FileExists(Path)
    local File = io.open(Path, "rb")
    if not File then
        return false
    end
    File:close()
    return true
end

local function AddCPath(Dir)
    if Dir and Dir ~= "" then
        package.cpath = package.cpath .. ";" .. Dir .. "/?.dll"
    end
end

local function TryLoadEmmy()
    local ScriptRoot = ""
    if UE and UE.UUnLuaFunctionLibrary and UE.UUnLuaFunctionLibrary.GetScriptRootPath then
        ScriptRoot = UE.UUnLuaFunctionLibrary.GetScriptRootPath()
    end

    local Dirs = {
        ScriptRoot .. "Debug/bin/x64",
        os.getenv("APPDATA") and (os.getenv("APPDATA"):gsub("\\", "/") .. "/JetBrains/IdeaIC2024.1/plugins/EmmyLua/debugger/emmy/windows/x64") or "",
        os.getenv("APPDATA") and (os.getenv("APPDATA"):gsub("\\", "/") .. "/JetBrains/IdeaIC2024.2/plugins/EmmyLua/debugger/emmy/windows/x64") or "",
        os.getenv("APPDATA") and (os.getenv("APPDATA"):gsub("\\", "/") .. "/JetBrains/IdeaIC2024.3/plugins/EmmyLua/debugger/emmy/windows/x64") or "",
        os.getenv("APPDATA") and (os.getenv("APPDATA"):gsub("\\", "/") .. "/JetBrains/IdeaIC2025.1/plugins/EmmyLua/debugger/emmy/windows/x64") or "",
        os.getenv("APPDATA") and (os.getenv("APPDATA"):gsub("\\", "/") .. "/JetBrains/IntelliJIdea2024.1/plugins/EmmyLua/debugger/emmy/windows/x64") or "",
    }

    for _, Dir in ipairs(Dirs) do
        if Dir ~= "" and FileExists(Dir .. "/emmy_core.dll") then
            AddCPath(Dir)
            break
        end
    end

    local Ok, Dbg = pcall(require, "emmy_core")
    if not Ok or not Dbg then
        UnLua.LogWarn("[EmmyDebug] emmy_core.dll not found. Copy it to Content/Script/Debug/bin/x64/")
        return
    end

    local ConnectOk, ConnectErr = pcall(function()
        Dbg.tcpConnect(HOST, PORT)
    end)
    if ConnectOk then
        UnLua.Log(string.format("[EmmyDebug] connected to IDEA %s:%d", HOST, PORT))
    else
        UnLua.Log(string.format("[EmmyDebug] IDEA not listening on %s:%d (%s). Start 'UnLua Emmy' in IDEA then PIE again.", HOST, PORT, tostring(ConnectErr)))
    end
end

local Ok, Err = pcall(TryLoadEmmy)
if not Ok then
    UnLua.LogWarn("[EmmyDebug] init failed: " .. tostring(Err))
end
