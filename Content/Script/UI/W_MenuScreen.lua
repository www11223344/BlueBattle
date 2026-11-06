--
-- 主菜单（L_Room_x 的 Menu 层根界面）
--

local UIFlow = require("UI.UIFlow")

---@type W_MenuScreen_C
local M = UnLua.Class()

function M:OnInitialized()
    UIFlow.BindClick(self, self.Play, M.OnPlayClicked)
    UIFlow.BindClick(self, self.Scenes, M.OnScenesClicked)
    UIFlow.BindClick(self, self.Gallery, M.OnGalleryClicked)
    UIFlow.BindClick(self, self.Settings, M.OnSettingsClicked)
    UIFlow.BindClick(self, self.Credits, M.OnCreditsClicked)
    UIFlow.BindClick(self, self.ExitGame, M.OnExitGameClicked)
    UIFlow.BindClick(self, self.WebPage, M.OnWebPageClicked)
    UIFlow.BindClick(self, self.ToolsPack, M.OnToolsPackClicked)
end

function M:OnPlayClicked()
    UIFlow.Push(self, "Menu", UIFlow.Screens.Play)
end

function M:OnScenesClicked()
    UIFlow.Push(self, "Menu", UIFlow.Screens.Scenes)
end

function M:OnGalleryClicked()
    UIFlow.Push(self, "Menu", UIFlow.Screens.Gallery)
end

function M:OnSettingsClicked()
    UIFlow.Push(self, "Menu", UIFlow.Screens.Settings)
end

function M:OnCreditsClicked()
    UIFlow.Push(self, "Menu", UIFlow.Screens.Credits)
end

function M:OnExitGameClicked()
    UE.UBlueBattleUIFunctionLibrary.RequestQuitGame(self, "退出游戏", "确定要退出游戏吗？")
end

function M:OnWebPageClicked()
    UE.UKismetSystemLibrary.LaunchURL(UIFlow.WebPageURL)
end

function M:OnToolsPackClicked()
    -- 原蓝图的 LaunchURL 未填地址
end

return M
