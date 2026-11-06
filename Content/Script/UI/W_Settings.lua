--
-- 设置（主菜单与暂停菜单共用）：Apply 按画质选项设置引擎画质，返回关闭
--

local UIFlow = require("UI.UIFlow")

---@type W_Settings_C
local M = UnLua.Class()

-- Quality 选项序号 0..4 对应"相对最高画质的降级档位" 4..0
local MAX_QUALITY_INDEX = 4

function M:OnInitialized()
    UIFlow.BindClick(self, self.Apply, M.OnApplyClicked)
    UIFlow.BindClick(self, self.Back, M.OnBackClicked)
end

function M:OnApplyClicked()
    local QualityIndex = self.Quality.ActualIndex
    if QualityIndex >= 0 and QualityIndex <= MAX_QUALITY_INDEX then
        UE.UAutomationBlueprintFunctionLibrary.SetScalabilityQualityLevelRelativeToMax(self, MAX_QUALITY_INDEX - QualityIndex)
    end
    -- 音量选项原蓝图未实现
end

function M:OnBackClicked()
    self:DeactivateWidget()
end

return M
