/**************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Installer Framework.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
**
** $QT_END_LICENSE$
**
**************************************************************************/

function Component()
{
	console.log("30.09 --------------------------------");
	installer.installationFinished.connect(this, Component.prototype.installationFinishedPageIsShown);
    installer.finishButtonClicked.connect(this, Component.prototype.installationFinished);
	
    console.log("OS: " + systemInfo.productType);
    console.log("Kernel: " + systemInfo.kernelType + "/" + systemInfo.kernelVersion);
	console.log("CPU: " + systemInfo.currentCpuArchitecture);
		
	// do we have an x64 OS?
	if (systemInfo.currentCpuArchitecture === "x86_64") {
        installer.componentByName("nomacs.x64").setValue("Default", "true");
		installer.componentByName("nomacs.x86").setValue("Default", "false");
		
		var programFiles = installer.environmentVariable("ProgramFiles");
		
		// if (programFiles !== "") {
			// installer.setValue("TargetDir", programFiles + "/nomacs");
			// console.log("Target dir changed to program files: " + programFiles);
		// }
		// else
			// console.log("WARNING: programFiles is empty...");
    }
	else {
        installer.componentByName("nomacs.x64").setValue("Default", "false");
		installer.componentByName("nomacs.x86").setValue("Default", "true");
    }
	
	// hide ready for installation page:
	installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
}

Component.prototype.isDefault = function()
{
    // select the component by default
    return true;
}

Component.prototype.createOperations = function()
{
    try {
        // call the base create operations function
        component.createOperations();
	
		if (installer.componentByName("nomacs.x86").installationRequested()) {
			component.addOperation("CreateShortcut", "@TargetDir@/nomacs-x86/nomacs.exe",   "@StartMenuDir@/nomacs - Image Lounge [x86].lnk", "workingDirectory=@TargetDir@");
			component.addOperation("CreateShortcut", "@TargetDir@/nomacs-x86/nomacs.exe",   "@TargetDir@/nomacs - Image Lounge [x86].exe.lnk", "workingDirectory=@TargetDir@");
		}
		else if (installer.componentByName("nomacs.x64").installationRequested()) {
			component.addOperation("CreateShortcut", "@TargetDir@/nomacs-x64/nomacs.exe",   "@StartMenuDir@/nomacs - Image Lounge [x64].lnk", "workingDirectory=@TargetDir@");
			component.addOperation("CreateShortcut", "@TargetDir@/nomacs-x64/nomacs.exe",   "@TargetDir@/nomacs - Image Lounge [x64].exe.lnk", "workingDirectory=@TargetDir@");
		}
	} catch (e) {
        console.log(e);
    }

}

Component.prototype.loaded = function ()
{
    var page = gui.pageByObjectName("DynamicPage");
    if (page != null) {
        console.log("Connecting the dynamic page entered signal.");
        page.entered.connect(Component.prototype.dynamicPageEntered);
    }
}

Component.prototype.installationFinishedPageIsShown = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {
            installer.addWizardPageItem( component, "OpenAppCheckBoxForm", QInstaller.InstallationFinished );
        }
    } catch(e) {
        console.log(e);
    }
}

Component.prototype.installationFinished = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {

			// open nomacs
			var isOpenAppCheckBoxChecked = component.userInterface("OpenAppCheckBoxForm").openAppCheckBox.checked;
            if (isOpenAppCheckBoxChecked) {
				
				if (installer.componentByName("nomacs.x64").installationRequested())
					QDesktopServices.openUrl("file:///" + installer.value("TargetDir") + "/nomacs-x64/nomacs.exe");
				else if (installer.componentByName("nomacs.x86").installationRequested())
					QDesktopServices.openUrl("file:///" + installer.value("TargetDir") + "/nomacs-x86/nomacs.exe");
			}
        }
    } catch(e) {
        console.log(e);
    }
}
