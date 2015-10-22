function Controller()
{
	if (installer.isInstaller()) {
		installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
	} else if (installer.isUninstaller()) {
		installer.setDefaultPageVisible(QInstaller.Introduction, false);
		installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
		installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
	}
}

// Select by default "I accept" the Licence Agreement. Seriously, who doesn't?
Controller.prototype.LicenseAgreementPageCallback = function()
{
	var widget = gui.currentPageWidget();
	if (widget != null) {
		widget.AcceptLicenseRadioButton.checked = true;
	}
}
