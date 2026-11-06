--
-- 开场第二屏：图片/文字轮播由蓝图 Construct 驱动，显示 15 秒后进入主菜单房间
--

local UIFlow = require("UI.UIFlow")

---@type W_InitialLoadScreen_2_C
local M = UnLua.Class()

local DURATION = 15.0

function M:BP_OnActivated()
    UIFlow.RunAfter(self, DURATION, function()
        UIFlow.OpenLevel(self, UIFlow.Levels.Rooms[1])
    end)
end

return M
