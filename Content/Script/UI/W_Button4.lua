--
-- 画廊缩略图按钮：点击后全屏查看 BackgroundImage
--

---@type W_Button4_C
local M = UnLua.Class("UI.W_ArtButton")

local IMAGE_SCREEN_CLASS = "/Game/UI/Widgets/W_ImageScreen.W_ImageScreen_C"

function M:BP_OnClicked()
    self.Super.BP_OnClicked(self)

    local WidgetClass = UE.UClass.Load(IMAGE_SCREEN_CLASS)
    if not WidgetClass then
        return
    end

    require("UI.W_ImageScreen").PendingImage = self.BackgroundImage
    UE.UBlueBattleUIFunctionLibrary.PushScreen(self, "Modal", WidgetClass)
end

return M
