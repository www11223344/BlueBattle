--
-- 美术按钮（W_Button1/2/3/5 共用，W_Button4 继承）
-- 外层是 CommonButton（负责点击/悬停/手柄焦点），内部原有的 Button 只做显示（HitTestInvisible），
-- 这里按 CommonButton 的状态切换内部 Button 的笔刷与文字颜色，复现原来的悬停/按下表现。
--

---@type W_Button1_C
local M = UnLua.Class()

function M:Construct()
    -- PreConstruct 已把 BackgroundImage 写进内部 Button 的样式，这里缓存三态笔刷
    local Inner = self.Button
    if Inner then
        self.CachedStyle = Inner.WidgetStyle:Copy()
    end

    -- W_Button5 的按钮文字（控件与 CommonButton 的 ButtonText 属性重名，已改名 ButtonLabel）
    if self.ButtonLabel and self.ButtonText_Edit then
        self.ButtonLabel:SetText(self.ButtonText_Edit)
    end

    self.bHovered = false
    self.bFocused = false
    self.bPressed = false
    self.bUsingGamepad = UE.UCommonUIExtensions.IsOwningPlayerUsingGamepad(self)
    self:RefreshVisualState()
end

function M:RefreshVisualState()
    local State = "Normal"
    if self.bPressed then
        State = "Pressed"
    elseif self.bHovered or (self.bFocused and self.bUsingGamepad) then
        State = "Hovered"
    end
    if self.VisualState == State then
        return
    end
    self.VisualState = State

    local Inner = self.Button
    local Cached = self.CachedStyle
    if Inner and Cached then
        local Style = Cached:Copy()
        Style.Normal = Cached[State]
        Inner:SetStyle(Style)
    end

    -- 蓝图变量名带空格，只能用字符串下标访问
    local BaseColor = self["Base Color"]
    local HoverColor = self["Hover Color"]
    local PressedColor = self["Pressed Color"]
    local Label = self.Text
    if Label and BaseColor then
        local Color = BaseColor
        if State == "Hovered" then
            Color = HoverColor or BaseColor
        elseif State == "Pressed" then
            Color = PressedColor or HoverColor or BaseColor
        end
        Label:SetColorAndOpacity(Color)
    end
end

function M:BP_OnInputMethodChanged(CurrentInputType)
    self.bUsingGamepad = CurrentInputType == UE.ECommonInputType.Gamepad
    self:RefreshVisualState()
end

function M:BP_OnHovered()
    self.bHovered = true
    self:RefreshVisualState()
    if self.OnHonvered then
        self.OnHonvered:Broadcast()
    end
end

function M:BP_OnUnhovered()
    self.bHovered = false
    self.bPressed = false
    self:RefreshVisualState()
end

function M:BP_OnFocusReceived()
    self.bFocused = true
    self:RefreshVisualState()
end

function M:BP_OnFocusLost()
    self.bFocused = false
    self:RefreshVisualState()
end

function M:BP_OnPressed()
    self.bPressed = true
    self:RefreshVisualState()
end

function M:BP_OnReleased()
    self.bPressed = false
    self:RefreshVisualState()
end

function M:BP_OnClicked()
    -- 兼容旧界面绑定的 OnClick 事件分发器
    if self.OnClick then
        self.OnClick:Broadcast()
    end
end

return M
