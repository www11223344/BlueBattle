--
-- W_Credits runtime logic (bound via GetModuleName -> UI.W_Credits)
--

---@type W_Credits_C
local M = UnLua.Class()

local SCROLL_STEP = 0.5
local MENU_SCREEN_CLASS = "/Game/UI/Widgets/W_MenuScreen.W_MenuScreen_C"

local function BindBackClick(self)
    local Back = self.Back
    if not (Back and Back.OnClick) then
        return
    end

    Back.OnClick:Remove(self, M.OnBackClicked)
    Back.OnClick:Add(self, M.OnBackClicked)
end

function M:Construct()
    self.bAutoScrolling = true
    BindBackClick(self)
end

function M:Destruct()
    local Back = self.Back
    if Back and Back.OnClick then
        Back.OnClick:Remove(self, M.OnBackClicked)
    end
end

function M:Tick(MyGeometry, InDeltaTime)
    if not self.bAutoScrolling then
        return
    end

    local ScrollBox = self.ScrollBox_230
    if not ScrollBox then
        return
    end

    local Offset = self.ScrollOffset or 0.0
    ScrollBox:SetScrollOffset(Offset)
    Offset = Offset + SCROLL_STEP
    self.ScrollOffset = Offset

    if Offset >= ScrollBox:GetScrollOffsetOfEnd() - SCROLL_STEP then
        self.bAutoScrolling = false
    end
end

function M:OnBackClicked()
    local Player = self:GetOwningPlayer()
    local WidgetClass = UE.UClass.Load(MENU_SCREEN_CLASS)
    if not WidgetClass then
        return
    end

    -- UE5.8: Create Widget is still on WidgetBlueprintLibrary;
    -- RemoveAllWidgets moved to WidgetLayoutLibrary.
    local Menu
    local WidgetLibrary = UE.UWidgetBlueprintLibrary
    if WidgetLibrary and WidgetLibrary.Create then
        Menu = WidgetLibrary.Create(self, WidgetClass, Player)
    else
        Menu = NewObject(WidgetClass, Player or self)
        if Menu and Player then
            Menu:SetOwningPlayer(Player)
        end
    end

    UE.UWidgetLayoutLibrary.RemoveAllWidgets(self)

    if Menu then
        Menu:AddToViewport(0)
    end
end

return M
