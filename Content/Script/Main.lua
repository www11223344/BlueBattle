-- UnLua startup module (Project Settings -> Plugins -> UnLua -> Startup Module Name).
-- Loads EmmyLua debugger in editor so IDEA can hit breakpoints in Content/Script.

pcall(require, "Debug.EmmyDebug")
