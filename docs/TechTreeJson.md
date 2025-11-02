# Tech Tree File Format

The tech tree for Generals Zero Hour in GenHotkeys is built from the information in the `TechTree.json` file, rather than from the game's ini files, due to the simplification of information retrieval and data loading.

The tech tree description file is a JSON structure containing a list of factions called `TechTree`, each of which consists of the following fields: `ShortName`, `DisplayName`, `DisplayNameDescription`, and the following lists: `Buildings`, `Infantry`, `Vehicles`, `Aircraft`. Each element in these lists is also a structure with fields `Name` (defines which .webp file should be loaded from the `Resources\Icons` directory), `IngameName` (defines which string from the .csf file should be taken as the object name) and a list of lists `KeyboardLayouts`. These layouts represent the possible combinations of the GUI for a unit or building. Each element in a layout is a structure with two fields: `IconName` (which defines which .webp file should be loaded from the `Resources/Icons` directory) and `HotkeyString` (which defines which string from the .csf file should be used for the hotkey of the action).

Example:

```json
{
    "TechTree" : 
    [
        {
            "ShortName"               : "USA",
            "DisplayName"             : "USA",
            "DisplayNameDescription"  : "United Stated Of America",

            "ru" : 
            {
                "DisplayName"            : "США", 
                "DisplayNameDescription" : "Соединённые штаты Америки"
            },

            "Buildings" : 
            [
                {
                    "Name"            : "USAColdFusionReactor",
                    "IngameName"      : "OBJECT:ColdFusionReactor",
                    "KeyboardLayouts" : 
                    [
                        [
                            {"IconName" : "USAControlRods", "HotkeyString" : "CONTROLBAR:UpgradeAmericaAdvancedControlRods"},
                        ]
                    ]
                }
            ],

            "Infantry" : 
            [
                {
                    "Name"            : "USARanger",
                    "IngameName"      : "OBJECT:Ranger",
                    "KeyboardLayouts" : 
                    [
                        [
                            {"IconName" : "USAFlashbangs",       "HotkeyString" : "CONTROLBAR:FlashBangGrenadeMode"},
                        ]
                    ]
                }
            ],

            "Vehicles" : 
            [
                {
                    "Name"            : "USADozer",
                    "IngameName"      : "OBJECT:Dozer",
                    "KeyboardLayouts" : 
                    [
                        [
                            {"IconName" : "USAColdFusionReactor",    "HotkeyString" : "CONTROLBAR:ConstructAmericaPowerPlant"},
                        ]
                    ]
                }
            ],

            "Aircrafts" : 
            [
                {
                    "Name"            : "USAChinook",
                    "IngameName"      : "OBJECT:Chinook",
                    "KeyboardLayouts" : 
                    [
                        [
                            {"IconName" : "USACombatDrop", "HotkeyString" : "CONTROLBAR:CombatDrop"},
                        ]
                    ]
                }
            ],
        }
    ]
}
```
