/*******************************************************************************************************
 
 Welcome to the nomacs install script.
 
 You can run an unattended install by calling:
 nomacs-setup.exe --script auto-install.js
 
 In this script you change change options such as the target directory
 or which components to install on the client PC.
 
 Created on: 	02.11.2015
 Author:		Markus Diem
 
 *******************************************************************************************************/

function Controller()
{
    installer.setDefaultPageVisible(QInstaller.Introduction, true);
    installer.setDefaultPageVisible(QInstaller.ComponentSelection, true);
    installer.setDefaultPageVisible(QInstaller.PerformInstallation, true);
    installer.setDefaultPageVisible(QInstaller.InstallationFinished, true);
	installer.setDefaultPageVisible(QInstaller.StartMenuSelection, true);
	
	// pages we don't need
    installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
    installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
    installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
    installer.autoRejectMessageBoxes();

    installer.setMessageBoxAutomaticAnswer("OverwriteTargetDirectory", QMessageBox.Yes);
    installer.setMessageBoxAutomaticAnswer("stopProcessesForUpdates", QMessageBox.Ignore);
	
	// Choose a target dir -----------------------------------------------------
	// installer.setValue("TargetDir", "C:/nomacs");
}

// start installation - here the repo is checked
Controller.prototype.IntroductionPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

// choose which components to install
Controller.prototype.ComponentSelectionPageCallback = function() 
{
	// Select your components -----------------------------------------------------
	// var page = gui.currentPageWidget();
	// page.selectComponent("nomacs.x86");
	// page.deselectComponent("nomacs.x64");	
	// Select your components -----------------------------------------------------
	
	gui.clickButton(buttons.NextButton);
}

Controller.prototype.StartMenuDirectoryPageCallback = function () 
{
	gui.clickButton(buttons.NextButton);
}

// skip installation finished
Controller.prototype.PerformInstallationPageCallback = function()
{
    installer.setAutomatedPageSwitchEnabled(true);
    gui.clickButton(buttons.NextButton);
}

// close installer
Controller.prototype.FinishedPageCallback = function()
{
    gui.clickButton(buttons.FinishButton);
}
