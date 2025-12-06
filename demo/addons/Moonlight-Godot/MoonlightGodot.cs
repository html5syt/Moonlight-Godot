#if TOOLS
using Godot;

[Tool]
public partial class MoonlightGodot : EditorPlugin
{
    public override void _EnterTree()
    {
        GD.Print("Moonlight Godot Plugin: Entering tree.");
    }

    public override void _ExitTree()
    {
        GD.Print("Moonlight Godot Plugin: Exiting tree.");
    }
}
#endif
