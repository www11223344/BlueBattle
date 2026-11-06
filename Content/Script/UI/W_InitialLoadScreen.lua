--
-- 开场第一屏：显示 5 秒后切到第二屏
--

local UIFlow = require("UI.UIFlow")

---@type W_InitialLoadScreen_C
local M = UnLua.Class()

local DURATION = 5.0

function M:BP_OnActivated()
    UIFlow.RunAfter(self, DURATION, function()
        UIFlow.Push(self, "Menu", UIFlow.Screens.InitialLoad2)
        self:DeactivateWidget()
    end)
end

return M
