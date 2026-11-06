--
-- 全屏看图（Modal 层）。打开前由调用方写入 PendingImage，Close 按钮或返回键关闭。
--

---@type W_ImageScreen_C
local M = UnLua.Class()

M.PendingImage = nil

function M:Construct()
    local Texture = M.PendingImage or self.Image_View
    M.PendingImage = nil
    if Texture then
        self.Image_View = Texture
        self.Image:SetBrushResourceObject(Texture)
    end

    self.Close.OnClicked:Add(self, M.OnCloseClicked)
end

function M:Destruct()
    self.Close.OnClicked:Remove(self, M.OnCloseClicked)
end

function M:OnCloseClicked()
    self:DeactivateWidget()
end

return M
