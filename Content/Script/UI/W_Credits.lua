--
-- 制作人员：打开后自动滚动到底，返回关闭
--

local UIFlow = require("UI.UIFlow")

---@type W_Credits_C
local M = UnLua.Class()

local SCROLL_STEP = 0.5

function M:OnInitialized()
    UIFlow.BindClick(self, self.Back, M.OnBackClicked)
end

function M:BP_OnActivated()
    self.ScrollOffset = 0.0
    self.bAutoScrolling = true
end

function M:Tick(MyGeometry, InDeltaTime)
    if not self.bAutoScrolling then
        return
    end

    local ScrollBox = self.ScrollBox_230
    local Offset = self.ScrollOffset or 0.0
    ScrollBox:SetScrollOffset(Offset)
    Offset = Offset + SCROLL_STEP
    self.ScrollOffset = Offset

    if Offset >= ScrollBox:GetScrollOffsetOfEnd() - SCROLL_STEP then
        self.bAutoScrolling = false
    end
end

function M:OnBackClicked()
    self:DeactivateWidget()
end

return M
