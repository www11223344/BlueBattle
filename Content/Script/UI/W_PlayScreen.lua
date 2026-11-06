--
-- 进入对战前的过场：显示加载图标 5 秒后打开对战关卡
--

local UIFlow = require("UI.UIFlow")

---@type W_PlayScreen_C
local M = UnLua.Class()

local DURATION = 5.0

function M:BP_OnActivated()
    -- 控件不能叫 Load：UnLua 的 UObject.Load 会先于控件变量被查到
    self.LoadingIcon:SetVisibility(UE.ESlateVisibility.Visible)
    UIFlow.RunAfter(self, DURATION, function()
        self.LoadingIcon:SetVisibility(UE.ESlateVisibility.Hidden)
        UIFlow.OpenLevel(self, UIFlow.Levels.Battle)
    end)
end

return M
