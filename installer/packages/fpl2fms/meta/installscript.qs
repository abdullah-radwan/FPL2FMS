function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows")
	{
        component.addOperation("CreateShortcut", "@TargetDir@/FPL2FMS.exe", "@StartMenuDir@/FPL2FMS.lnk",
            "workingDirectory=@TargetDir@");
			
        component.addOperation("CreateShortcut", "@TargetDir@/FPL2FMS.exe", "@DesktopDir@/FPL2FMS.lnk",
            "workingDirectory=@TargetDir@");
    }
	else
	{
		component.addElevatedOperation("Execute", "chmod", "-R", "755", "@TargetDir@");
		
		component.addElevatedOperation("CreateDesktopEntry", 
                                  "/usr/share/applications/FPL2FMS.desktop", 
                                  'Type=Application\nTerminal=false\nExec="@TargetDir@/run.sh"\nName=FPL2FMS\nIcon=@TargetDir@/icon.svg\nCategories=Utility;');
	}
}
