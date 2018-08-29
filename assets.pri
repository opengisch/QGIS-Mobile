################################################
# DON'T EDIT THIS FILE, USE config.pri instead #
################################################

system( mkdir -p $$shadowed($$PWD)/tmp )
CURRENT_PATH = $$shadowed($$PWD)
system( "cd $$QGIS_INSTALL_PATH$$; rm $$shadowed($$PWD)/tmp/assets.zip 2>/dev/null; zip -r $$shadowed($$PWD)/tmp/assets.zip share/resources/ share/svg/; cd $$CURRENT_PATH$$" )
qgis_assets.path = /assets
qgis_assets.files = "$$shadowed($$PWD)/tmp/assets.zip"
INSTALLS += qgis_assets
qgis_providers.path = /libs/$$ANDROID_TARGET_ARCH$$
qgis_providers.files = $$files( $$OSGEO4A_DIR/$$ANDROID_TARGET_ARCH$$/lib*provider.so )
INSTALLS += qgis_providers
