// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BlueBattleWidgetEditorLibrary.generated.h"

class UWidget;
class UWidgetBlueprint;

/**
 * 编辑器脚本（Python）用的 Widget 蓝图层级编辑工具。
 * UWidgetBlueprint::WidgetTree 在 Python 中不可访问，这里提供查找/构造/替换/删除控件的入口。
 * 修改后需调用 BlueprintEditorLibrary.compile_blueprint 并保存资产。
 */
UCLASS()
class UBlueBattleWidgetEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 以缩进文本列出控件层级：名称 [类] slot=插槽类 */
	UFUNCTION(BlueprintCallable, Category = "BlueBattle|WidgetEditor")
	static FString DumpWidgetTree(UWidgetBlueprint* WidgetBlueprint);

	UFUNCTION(BlueprintCallable, Category = "BlueBattle|WidgetEditor")
	static UWidget* GetRootWidget(UWidgetBlueprint* WidgetBlueprint);

	UFUNCTION(BlueprintCallable, Category = "BlueBattle|WidgetEditor")
	static UWidget* FindWidget(UWidgetBlueprint* WidgetBlueprint, FName WidgetName);

	UFUNCTION(BlueprintCallable, Category = "BlueBattle|WidgetEditor")
	static TArray<UWidget*> GetAllWidgets(UWidgetBlueprint* WidgetBlueprint);

	/** 在蓝图的 WidgetTree 中构造一个尚未挂到层级上的控件 */
	UFUNCTION(BlueprintCallable, Category = "BlueBattle|WidgetEditor")
	static UWidget* ConstructWidget(UWidgetBlueprint* WidgetBlueprint, TSubclassOf<UWidget> WidgetClass, FName WidgetName);

	/** 把 Child 加到面板/内容控件 Parent 下（Index < 0 表示末尾，仅面板有效） */
	UFUNCTION(BlueprintCallable, Category = "BlueBattle|WidgetEditor")
	static bool AddChild(UWidgetBlueprint* WidgetBlueprint, UWidget* Parent, UWidget* Child, int32 Index = -1);

	/** 用 NewWidget 替换 OldWidget 在层级中的位置（保留插槽布局）；bMoveChildren 时把旧控件的子控件转移给新控件 */
	UFUNCTION(BlueprintCallable, Category = "BlueBattle|WidgetEditor")
	static bool ReplaceWidget(UWidgetBlueprint* WidgetBlueprint, UWidget* OldWidget, UWidget* NewWidget, bool bMoveChildren = true);

	/** 删除控件；bPromoteContent 时若其为只有一个子控件的内容控件，则子控件顶替其位置 */
	UFUNCTION(BlueprintCallable, Category = "BlueBattle|WidgetEditor")
	static bool RemoveWidget(UWidgetBlueprint* WidgetBlueprint, UWidget* Widget, bool bPromoteContent = true);

	UFUNCTION(BlueprintCallable, Category = "BlueBattle|WidgetEditor")
	static bool RenameWidget(UWidgetBlueprint* WidgetBlueprint, UWidget* Widget, FName NewName);

	/** 层级修改后标记蓝图结构变化（编译前调用） */
	UFUNCTION(BlueprintCallable, Category = "BlueBattle|WidgetEditor")
	static void MarkStructurallyModified(UWidgetBlueprint* WidgetBlueprint);
};
