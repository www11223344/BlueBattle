--
-- 画廊：缩略图按钮（W_Button4）自行打开大图，这里只处理返回
--

local UIFlow = require("UI.UIFlow")

---@type W_GalleryScreen_C
local M = UnLua.Class()

function M:OnInitialized()
    UIFlow.BindClick(self, self.Back, M.OnBackClicked)
end

function M:OnBackClicked()
    self:DeactivateWidget()
end

return M
