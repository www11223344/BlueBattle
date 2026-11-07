// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Debug/BlueBattleDebugWindow.h"

#include "BlueBattleGameplayTags.h"
#include "Blueprint/WidgetTree.h"
#include "Characters/WarriorCharacter.h"
#include "Combat/AttributeComponent.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Debug/BlueBattleDebugCommands.h"
#include "GameFramework/PlayerController.h"
#include "Input/CommonUIInputTypes.h"
#include "Mass/BlueBattleMassFragments.h"
#include "Mass/BlueBattleUnitSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleDebugWindow)

#define LOCTEXT_NAMESPACE "BlueBattleDebug"

namespace
{
	UTextBlock* MakeText(UWidgetTree* WidgetTree, const FText& Text, int32 FontSize, const FLinearColor& Color = FLinearColor::White)
	{
		UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>();
		FSlateFontInfo Font = TextBlock->GetFont();
		Font.Size = FontSize;
		TextBlock->SetFont(Font);
		TextBlock->SetColorAndOpacity(FSlateColor(Color));
		TextBlock->SetText(Text);
		return TextBlock;
	}

	UButton* MakeButton(UWidgetTree* WidgetTree, const FText& Text)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>();
		Button->AddChild(MakeText(WidgetTree, Text, 12, FLinearColor::Black));
		return Button;
	}

	UWidget* WrapWidth(UWidgetTree* WidgetTree, UWidget* Content, float Width)
	{
		USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>();
		SizeBox->SetWidthOverride(Width);
		SizeBox->AddChild(Content);
		return SizeBox;
	}

	void AddToRow(UHorizontalBox* Row, UWidget* Widget, bool bFill = false)
	{
		UHorizontalBoxSlot* RowSlot = Row->AddChildToHorizontalBox(Widget);
		RowSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
		RowSlot->SetVerticalAlignment(VAlign_Center);
		RowSlot->SetSize(FSlateChildSize(bFill ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic));
	}

	constexpr int32 NumSides = 2;
}

// ---- UBlueBattleDebugCommandRow ----

void UBlueBattleDebugCommandRow::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
	WidgetTree->RootWidget = Row;

	LabelText = MakeText(WidgetTree, FText::GetEmpty(), 14);
	AddToRow(Row, WrapWidth(WidgetTree, LabelText, 110.f));

	ValueTextBox = WidgetTree->ConstructWidget<UEditableTextBox>();
	AddToRow(Row, WrapWidth(WidgetTree, ValueTextBox, 120.f), true);

	UButton* ExecuteButton = MakeButton(WidgetTree, LOCTEXT("Execute", "执行"));
	ExecuteButton->OnClicked.AddDynamic(this, &ThisClass::HandleExecuteClicked);
	AddToRow(Row, ExecuteButton);
}

void UBlueBattleDebugCommandRow::Setup(const FBlueBattleDebugCommandEntry& InEntry, int32 InSide)
{
	Entry = InEntry;
	Side = InSide;
	LabelText->SetText(Entry.Label);
	ValueTextBox->SetText(FText::FromString(Entry.DefaultValue));
	ValueTextBox->SetHintText(Entry.ValueHint);
}

void UBlueBattleDebugCommandRow::HandleExecuteClicked()
{
	const FString Value = ValueTextBox->GetText().ToString().TrimStartAndEnd();
	if (!Value.IsNumeric())
	{
		OnExecuted.ExecuteIfBound(FString::Printf(TEXT("%s %s：请输入数字"), *BlueBattleDebug::GetSideName(Side), *Entry.Label.ToString()));
		return;
	}

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	const FString Command = FString::Printf(TEXT("%s %d %s"), *Entry.Command, Side, *Value);
	const FString Result = PlayerController->ConsoleCommand(Command).TrimStartAndEnd();
	OnExecuted.ExecuteIfBound(Result.IsEmpty() ? Command : Result);
}

// ---- UBlueBattleDebugWindow ----

UBlueBattleDebugWindow::UBlueBattleDebugWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputConfig = EBlueBattleWidgetInputMode::GameAndMenu;
	GameMouseCaptureMode = EMouseCaptureMode::NoCapture;
	bIsBackHandler = true;
	bSetVisibilityOnActivated = true;
	ActivatedVisibility = ESlateVisibility::SelfHitTestInvisible;
	bSetVisibilityOnDeactivated = true;
	DeactivatedVisibility = ESlateVisibility::Collapsed;

	auto AddCommand = [this](const FText& Label, const TCHAR* Command, const TCHAR* DefaultValue, const FText& ValueHint)
	{
		FBlueBattleDebugCommandEntry& Entry = Commands.AddDefaulted_GetRef();
		Entry.Label = Label;
		Entry.Command = Command;
		Entry.DefaultValue = DefaultValue;
		Entry.ValueHint = ValueHint;
	};
	AddCommand(LOCTEXT("SetHP", "生命值"), TEXT("BB.SetHP"), TEXT("100"), LOCTEXT("SetHPHint", "超过上限会抬高上限"));
	AddCommand(LOCTEXT("SetDamage", "每次命中伤害"), TEXT("BB.SetDamage"), TEXT("-1"), LOCTEXT("SetDamageHint", "-1 = 按招式表"));
}

void UBlueBattleDebugWindow::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetVisibility(DeactivatedVisibility);

	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>();
	Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = Root;

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>();
	Panel->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.75f));
	Panel->SetPadding(FMargin(12.f));
	UOverlaySlot* PanelSlot = Root->AddChildToOverlay(Panel);
	PanelSlot->SetHorizontalAlignment(HAlign_Left);
	PanelSlot->SetVerticalAlignment(VAlign_Top);
	PanelSlot->SetPadding(FMargin(24.f, 160.f, 0.f, 0.f));

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>();
	Panel->SetContent(Content);
	auto AddLine = [Content](UWidget* Widget)
	{
		Content->AddChildToVerticalBox(Widget)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
	};

	UHorizontalBox* TitleRow = WidgetTree->ConstructWidget<UHorizontalBox>();
	AddToRow(TitleRow, MakeText(WidgetTree, LOCTEXT("Title", "调试窗口 (K)"), 16, FLinearColor(1.f, 0.8f, 0.2f)), true);
	UButton* CloseButton = MakeButton(WidgetTree, LOCTEXT("Close", "关闭"));
	CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	AddToRow(TitleRow, CloseButton);
	AddLine(TitleRow);

	for (int32 Side = 0; Side < NumSides; ++Side)
	{
		AddLine(MakeText(WidgetTree, FText::FromString(BlueBattleDebug::GetSideName(Side)), 15, FLinearColor(0.4f, 0.8f, 1.f)));

		UTextBlock* SideStatusText = MakeText(WidgetTree, FText::GetEmpty(), 13);
		StatusTexts.Add(SideStatusText);
		AddLine(SideStatusText);

		for (const FBlueBattleDebugCommandEntry& Entry : Commands)
		{
			UBlueBattleDebugCommandRow* Row = CreateWidget<UBlueBattleDebugCommandRow>(this);
			Row->Setup(Entry, Side);
			Row->OnExecuted.BindUObject(this, &ThisClass::HandleCommandExecuted);
			AddLine(Row);
		}
	}

	ResultText = MakeText(WidgetTree, FText::GetEmpty(), 12, FLinearColor(0.6f, 1.f, 0.6f));
	ResultText->SetAutoWrapText(true);
	AddLine(WrapWidth(WidgetTree, ResultText, 420.f));

	RefreshStatus();
}

void UBlueBattleDebugWindow::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	StatusRefreshCountdown -= InDeltaTime;
	if (StatusRefreshCountdown <= 0.f)
	{
		StatusRefreshCountdown = StatusRefreshInterval;
		RefreshStatus();
	}
}

void UBlueBattleDebugWindow::HandleCloseClicked()
{
	DeactivateWidget();
}

void UBlueBattleDebugWindow::HandleCommandExecuted(const FString& Result)
{
	ResultText->SetText(FText::FromString(Result));
	RefreshStatus();
}

void UBlueBattleDebugWindow::RefreshStatus()
{
	UWorld* World = GetWorld();
	const UBlueBattleUnitSubsystem* UnitSubsystem = World ? World->GetSubsystem<UBlueBattleUnitSubsystem>() : nullptr;

	for (int32 Side = 0; Side < StatusTexts.Num(); ++Side)
	{
		const TArray<AWarriorCharacter*> Warriors = BlueBattleDebug::FindWarriors(World, Side);
		const UAttributeComponent* AttributeComponent = Warriors.IsEmpty() ? nullptr : Warriors[0]->GetAttributeComponent();
		const FBlueBattleAttributeFragment* Attributes = (UnitSubsystem && AttributeComponent) ? UnitSubsystem->GetAttributes(AttributeComponent->GetMassEntity()) : nullptr;
		if (!Attributes)
		{
			StatusTexts[Side]->SetText(LOCTEXT("NotReady", "未就绪"));
			continue;
		}

		const FString DamageText = Attributes->DamageOverride >= 0.f ? FString::Printf(TEXT("%g"), Attributes->DamageOverride) : FString(TEXT("招式表"));
		StatusTexts[Side]->SetText(FText::FromString(FString::Printf(TEXT("生命 %.0f / %.0f    每次命中伤害 %s"), Attributes->HP, Attributes->MaxHP, *DamageText)));
	}
}

// ---- UBlueBattleDebugHUDEntry ----

UBlueBattleDebugHUDEntry::UBlueBattleDebugHUDEntry(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DebugWindowClass = UBlueBattleDebugWindow::StaticClass();
}

void UBlueBattleDebugHUDEntry::NativeOnInitialized()
{
	Super::NativeOnInitialized();

#if !UE_BUILD_SHIPPING
	if (!DebugWindowClass)
	{
		return;
	}

	DebugWindow = CreateWidget<UBlueBattleDebugWindow>(this, DebugWindowClass);
	WidgetTree->RootWidget = DebugWindow;

	// 窗口激活后路由器仍先处理子节点、再处理本绑定所属的 HUD 布局节点，All 模式下 Game 绑定同样生效，所以开关共用这一个绑定
	FBindUIActionArgs ToggleActionArgs(FUIActionTag::ConvertChecked(BlueBattleGameplayTags::UI_Action_DebugWindow), false,
		FSimpleDelegate::CreateUObject(this, &ThisClass::HandleToggleAction));
	ToggleActionArgs.InputMode = ECommonInputMode::Game;
	RegisterUIActionBinding(ToggleActionArgs);
#endif
}

void UBlueBattleDebugHUDEntry::HandleToggleAction()
{
	if (DebugWindow->IsActivated())
	{
		DebugWindow->DeactivateWidget();
	}
	else
	{
		DebugWindow->ActivateWidget();
	}
}

#undef LOCTEXT_NAMESPACE
