--
-- 进入对战前的过场：显示加载图标 5 秒后打开对战关卡
--

local UIFlow = require("UI.UIFlow")

---@type W_PlayScreen_C
local M = UnLua.Class()

local DURATION = 5.0

function M:BP_OnActivated()
    self.Load:SetVisibility(UE.ESlateVisibility.Visible)
    UIFlow.RunAfter(self, DURATION, function()
        self.Load:SetVisibility(UE.ESlateVisibility.Hidden)
        UIFlow.OpenLevel(self, UIFlow.Levels.Battle)
    end)
end

return M
