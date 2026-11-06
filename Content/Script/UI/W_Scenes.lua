--
-- 场景选择：四个房间按钮切换主菜单房间
--

local UIFlow = require("UI.UIFlow")

---@type W_Scenes_C
local M = UnLua.Class()

local ROOM_BUTTONS = { "Button5_W", "Button5_W_1", "Button5_W_2", "Button5_W_3" }

function M:OnInitialized()
    for Index, ButtonName in ipairs(ROOM_BUTTONS) do
        UIFlow.BindClick(self, self[ButtonName], function(Self)
            UIFlow.OpenLevel(Self, UIFlow.Levels.Rooms[Index])
        end)
    end
    UIFlow.BindClick(self, self.Back, M.OnBackClicked)
end

function M:OnBackClicked()
    self:DeactivateWidget()
end

return M
