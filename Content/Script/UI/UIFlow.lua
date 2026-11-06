--
-- UI 流程公共函数：推界面到 PrimaryGameLayout 的层、延时回调、切关卡、绑定 CommonButton 点击
--

local UIFlow = {}

UIFlow.Screens = {
    InitialLoad2 = "/Game/UI/Widgets/W_InitialLoadScreen_2.W_InitialLoadScreen_2_C",
    Play = "/Game/UI/Widgets/W_PlayScreen.W_PlayScreen_C",
    Scenes = "/Game/UI/Widgets/W_Scenes.W_Scenes_C",
    Gallery = "/Game/UI/Widgets/W_GalleryScreen.W_GalleryScreen_C",
    Credits = "/Game/UI/Widgets/W_Credits.W_Credits_C",
    Settings = "/Game/UI/Widgets/W_Settings.W_Settings_C",
}

UIFlow.Levels = {
    InitialMap = "/Game/UI/Maps/L_InitialMap",
    Battle = "/Game/ice",
    Rooms = { "/Game/UI/Maps/L_Room_1", "/Game/UI/Maps/L_Room_2", "/Game/UI/Maps/L_Room_3", "/Game/UI/Maps/L_Room_4" },
}

UIFlow.WebPageURL = "https://space.bilibili.com/166612188?spm_id_from=333.1007.0.0"

--- 把界面推到 Context 所属玩家的 UI 层（Layer: "Game" / "GameMenu" / "Menu" / "Modal"）
function UIFlow.Push(Context, Layer, ClassPath)
    local WidgetClass = UE.UClass.Load(ClassPath)
    if not WidgetClass then
        UnLua.LogError("UIFlow.Push: class not found " .. tostring(ClassPath))
        return nil
    end
    return UE.UBlueBattleUIFunctionLibrary.PushScreen(Context, Layer, WidgetClass)
end

--- Seconds 秒后调用 Callback；Context 销毁则不再回调
function UIFlow.RunAfter(Context, Seconds, Callback)
    coroutine.resume(coroutine.create(function()
        UE.UKismetSystemLibrary.Delay(Context, Seconds)
        Callback()
    end))
end

function UIFlow.OpenLevel(Context, LevelPath)
    UE.UGameplayStatics.OpenLevel(Context, LevelPath, true, "")
end

--- 绑定 CommonButton 点击；Handler 以 (Owner) 调用
function UIFlow.BindClick(Owner, Button, Handler)
    if not Button then
        return
    end
    Button.OnButtonBaseClicked:Add(Owner, function(Self)
        Handler(Self)
    end)
end

return UIFlow
