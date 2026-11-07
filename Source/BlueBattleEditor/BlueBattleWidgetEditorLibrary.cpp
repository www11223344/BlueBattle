// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueBattleWidgetEditorLibrary.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ContentWidget.h"
#include "Components/PanelSlot.h"
#include "Components/PanelWidget.h"
#include "Editor.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "MVVMBlueprintView.h"
#include "MVVMBlueprintViewBinding.h"
#include "MVVMBlueprintViewModelContext.h"
#include "MVVMEditorSubsystem.h"
#include "MVVMPropertyPath.h"
#include "Types/MVVMFieldVariant.h"
#include "View/MVVMViewModelContextResolver.h"
#include "UObject/Package.h"
#include "WidgetBlueprint.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleWidgetEditorLibrary)

namespace BlueBattleWidgetEditor
{
	static void DumpRecursive(const UWidget* Widget, int32 Depth, FString& Out)
	{
		if (!Widget)
		{
			return;
		}

		Out += FString::ChrN(Depth * 2, TEXT(' '));
		Out += FString::Printf(TEXT("%s [%s]"), *Widget->GetName(), *Widget->GetClass()->GetName());
		if (Widget->Slot)
		{
			Out += FString::Printf(TEXT(" slot=%s"), *Widget->Slot->GetClass()->GetName());
		}
		Out += TEXT("\n");

		if (const UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
		{
			for (int32 Index = 0; Index < Panel->GetChildrenCount(); ++Index)
			{
				DumpRecursive(Panel->GetChildAt(Index), Depth + 1, Out);
			}
		}
	}

	/** 复制插槽布局属性（跳过 UPanelSlot 自身记录父子关系的字段） */
	static void CopySlotLayout(const UPanelSlot* From, UPanelSlot* To)
	{
		if (!From || !To || From->GetClass() != To->GetClass())
		{
			return;
		}

		for (TFieldIterator<FProperty> It(From->GetClass()); It; ++It)
		{
			if (It->GetOwnerClass() == UPanelSlot::StaticClass() || It->HasAnyPropertyFlags(CPF_Transient))
			{
				continue;
			}
			It->CopyCompleteValue_InContainer(To, From);
		}
	}

	static void DiscardWidget(UWidgetBlueprint* WidgetBlueprint, UWidget* Widget)
	{
		const FName OldName = Widget->GetFName();
		WidgetBlueprint->WidgetTree->RemoveWidget(Widget);
		Widget->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_NonTransactional);
		WidgetBlueprint->OnVariableRemoved(OldName);
	}
}

FString UBlueBattleWidgetEditorLibrary::DumpWidgetTree(UWidgetBlueprint* WidgetBlueprint)
{
	FString Out;
	if (WidgetBlueprint && WidgetBlueprint->WidgetTree)
	{
		BlueBattleWidgetEditor::DumpRecursive(WidgetBlueprint->WidgetTree->RootWidget, 0, Out);
	}
	return Out;
}

UWidget* UBlueBattleWidgetEditorLibrary::GetRootWidget(UWidgetBlueprint* WidgetBlueprint)
{
	return (WidgetBlueprint && WidgetBlueprint->WidgetTree) ? WidgetBlueprint->WidgetTree->RootWidget.Get() : nullptr;
}

bool UBlueBattleWidgetEditorLibrary::SetRootWidget(UWidgetBlueprint* WidgetBlueprint, UWidget* Widget)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !Widget)
	{
		return false;
	}

	WidgetBlueprint->WidgetTree->Modify();
	Widget->RemoveFromParent();
	WidgetBlueprint->WidgetTree->RootWidget = Widget;
	return true;
}

UWidget* UBlueBattleWidgetEditorLibrary::FindWidget(UWidgetBlueprint* WidgetBlueprint, FName WidgetName)
{
	return (WidgetBlueprint && WidgetBlueprint->WidgetTree) ? WidgetBlueprint->WidgetTree->FindWidget(WidgetName) : nullptr;
}

TArray<UWidget*> UBlueBattleWidgetEditorLibrary::GetAllWidgets(UWidgetBlueprint* WidgetBlueprint)
{
	TArray<UWidget*> Widgets;
	if (WidgetBlueprint && WidgetBlueprint->WidgetTree)
	{
		WidgetBlueprint->WidgetTree->GetAllWidgets(Widgets);
	}
	return Widgets;
}

UWidget* UBlueBattleWidgetEditorLibrary::ConstructWidget(UWidgetBlueprint* WidgetBlueprint, TSubclassOf<UWidget> WidgetClass, FName WidgetName)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !WidgetClass)
	{
		return nullptr;
	}

	WidgetBlueprint->WidgetTree->Modify();
	UWidget* Widget = WidgetBlueprint->WidgetTree->ConstructWidget<UWidget>(WidgetClass, WidgetName);
	WidgetBlueprint->OnVariableAdded(Widget->GetFName());
	return Widget;
}

bool UBlueBattleWidgetEditorLibrary::AddChild(UWidgetBlueprint* WidgetBlueprint, UWidget* Parent, UWidget* Child, int32 Index)
{
	if (!WidgetBlueprint || !Child)
	{
		return false;
	}

	if (UContentWidget* ContentParent = Cast<UContentWidget>(Parent))
	{
		return ContentParent->SetContent(Child) != nullptr;
	}
	if (UPanelWidget* PanelParent = Cast<UPanelWidget>(Parent))
	{
		return (Index < 0 ? PanelParent->AddChild(Child) : PanelParent->InsertChildAt(Index, Child)) != nullptr;
	}
	return false;
}

bool UBlueBattleWidgetEditorLibrary::ReplaceWidget(UWidgetBlueprint* WidgetBlueprint, UWidget* OldWidget, UWidget* NewWidget, bool bMoveChildren)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !OldWidget || !NewWidget || OldWidget == NewWidget)
	{
		return false;
	}

	UWidgetTree* WidgetTree = WidgetBlueprint->WidgetTree;
	WidgetTree->Modify();

	if (bMoveChildren)
	{
		UPanelWidget* OldPanel = Cast<UPanelWidget>(OldWidget);
		UPanelWidget* NewPanel = Cast<UPanelWidget>(NewWidget);
		if (OldPanel && NewPanel)
		{
			TArray<UWidget*> Children = OldPanel->GetAllChildren();
			for (UWidget* Child : Children)
			{
				OldPanel->RemoveChild(Child);
				if (NewPanel->CanAddMoreChildren())
				{
					NewPanel->AddChild(Child);
				}
			}
		}
	}

	if (WidgetTree->RootWidget == OldWidget)
	{
		WidgetTree->RootWidget = NewWidget;
		OldWidget->RemoveFromParent();
	}
	else
	{
		UPanelWidget* Parent = OldWidget->GetParent();
		if (!Parent)
		{
			return false;
		}

		const int32 Index = Parent->GetChildIndex(OldWidget);
		UPanelSlot* OldSlot = OldWidget->Slot;
		Parent->RemoveChildAt(Index);
		UPanelSlot* NewSlot = Parent->InsertChildAt(Index, NewWidget);
		BlueBattleWidgetEditor::CopySlotLayout(OldSlot, NewSlot);
	}

	BlueBattleWidgetEditor::DiscardWidget(WidgetBlueprint, OldWidget);
	return true;
}

bool UBlueBattleWidgetEditorLibrary::RemoveWidget(UWidgetBlueprint* WidgetBlueprint, UWidget* Widget, bool bPromoteContent)
{
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree || !Widget)
	{
		return false;
	}

	if (bPromoteContent)
	{
		if (UContentWidget* ContentWidget = Cast<UContentWidget>(Widget))
		{
			if (UWidget* Content = ContentWidget->GetContent())
			{
				ContentWidget->SetContent(nullptr);
				return ReplaceWidget(WidgetBlueprint, Widget, Content, false);
			}
		}
	}

	WidgetBlueprint->WidgetTree->Modify();
	BlueBattleWidgetEditor::DiscardWidget(WidgetBlueprint, Widget);
	return true;
}

bool UBlueBattleWidgetEditorLibrary::RenameWidget(UWidgetBlueprint* WidgetBlueprint, UWidget* Widget, FName NewName)
{
	if (!WidgetBlueprint || !Widget || NewName.IsNone())
	{
		return false;
	}

	const FName OldName = Widget->GetFName();
	if (!Widget->Rename(*NewName.ToString(), nullptr, REN_Test))
	{
		return false;
	}
	Widget->Rename(*NewName.ToString());
	WidgetBlueprint->OnVariableRenamed(OldName, NewName);
	return true;
}

namespace BlueBattleWidgetEditor
{
	static FMVVMBlueprintViewModelContext* FindViewModelContext(UMVVMBlueprintView* View, const UClass* ViewModelClass)
	{
		if (!View || !ViewModelClass)
		{
			return nullptr;
		}
		for (const FMVVMBlueprintViewModelContext& Context : View->GetViewModels())
		{
			if (Context.GetViewModelClass() == ViewModelClass)
			{
				return View->FindViewModel(Context.GetViewModelId());
			}
		}
		return nullptr;
	}

	static TOptional<UE::MVVM::FMVVMConstFieldVariant> FindField(const UClass* Class, FName FieldName)
	{
		if (const UFunction* Function = Class->FindFunctionByName(FieldName))
		{
			return UE::MVVM::FMVVMConstFieldVariant(Function);
		}
		if (const FProperty* Property = FindFProperty<FProperty>(Class, FieldName))
		{
			return UE::MVVM::FMVVMConstFieldVariant(Property);
		}
		return {};
	}
}

bool UBlueBattleWidgetEditorLibrary::AddViewModelBinding(UWidgetBlueprint* WidgetBlueprint, UClass* ViewModelClass, FName ViewModelField, FName WidgetName, FName WidgetProperty)
{
	UMVVMEditorSubsystem* MVVMEditor = GEditor ? GEditor->GetEditorSubsystem<UMVVMEditorSubsystem>() : nullptr;
	UWidget* Widget = FindWidget(WidgetBlueprint, WidgetName);
	if (!MVVMEditor || !Widget)
	{
		return false;
	}

	UMVVMBlueprintView* View = MVVMEditor->RequestView(WidgetBlueprint);
	const FMVVMBlueprintViewModelContext* Context = BlueBattleWidgetEditor::FindViewModelContext(View, ViewModelClass);
	const TOptional<UE::MVVM::FMVVMConstFieldVariant> SourceField = BlueBattleWidgetEditor::FindField(ViewModelClass, ViewModelField);
	const TOptional<UE::MVVM::FMVVMConstFieldVariant> DestinationField = BlueBattleWidgetEditor::FindField(Widget->GetClass(), WidgetProperty);
	if (!Context || !SourceField || !DestinationField)
	{
		return false;
	}

	Widget->bIsVariable = true;

	FMVVMBlueprintPropertyPath SourcePath;
	SourcePath.SetViewModelId(Context->GetViewModelId());
	SourcePath.AppendPropertyPath(WidgetBlueprint, SourceField.GetValue());

	FMVVMBlueprintPropertyPath DestinationPath;
	DestinationPath.SetWidgetName(WidgetName);
	DestinationPath.AppendPropertyPath(WidgetBlueprint, DestinationField.GetValue());

	FMVVMBlueprintViewBinding& Binding = MVVMEditor->AddBinding(WidgetBlueprint);
	Binding.BindingType = EMVVMBindingMode::OneWayToDestination;
	MVVMEditor->SetSourcePathForBinding(WidgetBlueprint, Binding, SourcePath);
	MVVMEditor->SetDestinationPathForBinding(WidgetBlueprint, Binding, DestinationPath, false);
	return true;
}

int32 UBlueBattleWidgetEditorLibrary::RemoveViewModelBindingsToWidget(UWidgetBlueprint* WidgetBlueprint, FName WidgetName)
{
	UMVVMEditorSubsystem* MVVMEditor = GEditor ? GEditor->GetEditorSubsystem<UMVVMEditorSubsystem>() : nullptr;
	UMVVMBlueprintView* View = MVVMEditor && WidgetBlueprint ? MVVMEditor->GetView(WidgetBlueprint) : nullptr;
	if (!View)
	{
		return 0;
	}

	int32 Removed = 0;
	for (int32 Index = View->GetNumBindings() - 1; Index >= 0; --Index)
	{
		const FMVVMBlueprintViewBinding* Binding = View->GetBindingAt(Index);
		if (Binding && Binding->DestinationPath.GetWidgetName() == WidgetName)
		{
			MVVMEditor->RemoveBinding(WidgetBlueprint, *Binding);
			++Removed;
		}
	}
	return Removed;
}

bool UBlueBattleWidgetEditorLibrary::SetViewModelResolver(UWidgetBlueprint* WidgetBlueprint, UClass* ViewModelClass, UClass* ResolverClass)
{
	UMVVMEditorSubsystem* MVVMEditor = GEditor ? GEditor->GetEditorSubsystem<UMVVMEditorSubsystem>() : nullptr;
	if (!MVVMEditor || !ResolverClass || !ResolverClass->IsChildOf<UMVVMViewModelContextResolver>())
	{
		return false;
	}

	UMVVMBlueprintView* View = MVVMEditor->RequestView(WidgetBlueprint);
	FMVVMBlueprintViewModelContext* Context = BlueBattleWidgetEditor::FindViewModelContext(View, ViewModelClass);
	if (!Context)
	{
		return false;
	}

	View->Modify();
	Context->CreationType = EMVVMBlueprintViewModelContextCreationType::Resolver;
	Context->Resolver = NewObject<UMVVMViewModelContextResolver>(View, ResolverClass, NAME_None, RF_Transactional);
	return true;
}

TArray<FString> UBlueBattleWidgetEditorLibrary::GetPropertyBindings(UWidgetBlueprint* WidgetBlueprint)
{
	TArray<FString> Result;
	if (WidgetBlueprint)
	{
		for (const FDelegateEditorBinding& Binding : WidgetBlueprint->Bindings)
		{
			const FString Target = Binding.SourcePath.IsEmpty() ? Binding.FunctionName.ToString() : Binding.SourcePath.GetDisplayText().ToString();
			Result.Add(FString::Printf(TEXT("%s.%s -> %s"), *Binding.ObjectName, *Binding.PropertyName.ToString(), *Target));
		}
	}
	return Result;
}

int32 UBlueBattleWidgetEditorLibrary::RemovePropertyBindings(UWidgetBlueprint* WidgetBlueprint, FName WidgetName)
{
	if (!WidgetBlueprint)
	{
		return 0;
	}
	WidgetBlueprint->Modify();
	const int32 Removed = WidgetBlueprint->Bindings.RemoveAll([WidgetName](const FDelegateEditorBinding& Binding)
	{
		return WidgetName.IsNone() || Binding.ObjectName == WidgetName.ToString();
	});
	if (Removed > 0)
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	}
	return Removed;
}

void UBlueBattleWidgetEditorLibrary::MarkStructurallyModified(UWidgetBlueprint* WidgetBlueprint)
{
	if (WidgetBlueprint)
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);
	}
}

bool UBlueBattleWidgetEditorLibrary::SimulateMouseMoveToWidget(UWidget* Widget)
{
	if (!Widget || !FSlateApplication::IsInitialized())
	{
		return false;
	}

	const FGeometry& Geometry = Widget->GetCachedGeometry();
	const FVector2D Target = Geometry.LocalToAbsolute(Geometry.GetLocalSize() * 0.5f);

	FSlateApplication& SlateApp = FSlateApplication::Get();
	const FPointerEvent MouseEvent(
		SlateApp.GetUserIndexForMouse(),
		FSlateApplicationBase::CursorPointerIndex,
		Target,
		SlateApp.GetLastCursorPos(),
		SlateApp.GetPressedMouseButtons(),
		EKeys::Invalid,
		0.f,
		SlateApp.GetPlatformApplication()->GetModifierKeys());
	SlateApp.ProcessMouseMoveEvent(MouseEvent);

	return Widget->IsHovered();
}

bool UBlueBattleWidgetEditorLibrary::SimulateKeyPress(FKey Key, int32 UserIndex)
{
	if (!Key.IsValid() || !FSlateApplication::IsInitialized())
	{
		return false;
	}

	FSlateApplication& SlateApp = FSlateApplication::Get();
	const FModifierKeysState Modifiers = SlateApp.GetPlatformApplication()->GetModifierKeys();
	const FKeyEvent KeyEvent(Key, Modifiers, UserIndex, false, 0, 0);
	const bool bHandled = SlateApp.ProcessKeyDownEvent(KeyEvent);
	SlateApp.ProcessKeyUpEvent(KeyEvent);
	return bHandled;
}

bool UBlueBattleWidgetEditorLibrary::SimulateGameViewportKey(UObject* WorldContextObject, FKey Key)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	UGameViewportClient* ViewportClient = World ? World->GetGameViewport() : nullptr;
	if (!ViewportClient || !ViewportClient->Viewport || !Key.IsValid())
	{
		return false;
	}

	const FInputDeviceId Device = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
	const bool bHandled = ViewportClient->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1.f, -1, Device, false, ViewportClient->Viewport));
	ViewportClient->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0.f, -1, Device, false, ViewportClient->Viewport));
	return bHandled;
}
