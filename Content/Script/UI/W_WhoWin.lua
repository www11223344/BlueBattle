--
-- 结算界面：胜者文字/图标由 MatchResult ViewModel 绑定，延时后提示按空格（手柄 A）返回开场关卡
--

local UIFlow = require("UI.UIFlow")

---@type W_WhoWin_C
local M = UnLua.Class()

local CONTINUE_DELAY = 3.0
local UNIVERSAL_ACTIONS = "/Game/UI/DT_UniversalActions.DT_UniversalActions"

function M:BP_OnActivated()
    self.Text_Press:SetText("")
    self.Loading:SetVisibility(UE.ESlateVisibility.Visible)
    UIFlow.RunAfter(self, CONTINUE_DELAY, function()
        self.Text_Press:SetText("轻按空格键以继续")
        self.Loading:SetVisibility(UE.ESlateVisibility.Hidden)

        local Action = UE.FDataTableRowHandle()
        Action.DataTable = UE.UObject.Load(UNIVERSAL_ACTIONS)
        Action.RowName = "Continue"
        self:ListenForUIAction(Action, "Continue", false)
    end)
end

function M:OnUIAction(ActionId)
    if ActionId == "Continue" then
        self:StopListeningForUIActions()
        UIFlow.OpenLevel(self, UIFlow.Levels.InitialMap)
    end
end

return M
