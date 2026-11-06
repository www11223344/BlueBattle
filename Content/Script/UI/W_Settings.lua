--
-- 设置（主菜单与暂停菜单共用）：Apply 按画质选项设置并保存引擎画质，返回关闭
-- 界面会被控件栈复用，每次激活都按当前生效的画质刷新选项，丢弃未应用的修改
--

local UIFlow = require("UI.UIFlow")

---@type W_Settings_C
local M = UnLua.Class()

-- Quality 选项序号 0..4（低..最高）对应 Scalability 档位 0..4
local MAX_QUALITY_INDEX = 4

function M:OnInitialized()
    UIFlow.BindClick(self, self.Apply, M.OnApplyClicked)
    UIFlow.BindClick(self, self.Back, M.OnBackClicked)
end

function M:BP_OnActivated()
    self:SetQualityIndex(M.GetCurrentQualityIndex())
end

function M.GetCurrentQualityIndex()
    local Settings = UE.UGameUserSettings.GetGameUserSettings()
    local Level = Settings:GetOverallScalabilityLevel()
    -- 各分组档位不一致时整体档位为 -1，以阴影档位为准
    if Level < 0 then
        Level = Settings:GetShadowQuality()
    end
    return math.max(0, math.min(Level, MAX_QUALITY_INDEX))
end

function M:SetQualityIndex(Index)
    self.Quality.ActualIndex = Index
    self.Quality.ComboBoxString_94:SetSelectedIndex(Index)
end

function M:OnApplyClicked()
    local QualityIndex = self.Quality.ActualIndex
    if QualityIndex >= 0 and QualityIndex <= MAX_QUALITY_INDEX then
        local Settings = UE.UGameUserSettings.GetGameUserSettings()
        Settings:SetOverallScalabilityLevel(QualityIndex)
        Settings:ApplySettings(false)
    end
    -- 音量选项原蓝图未实现
end

function M:OnBackClicked()
    self:DeactivateWidget()
end

return M
