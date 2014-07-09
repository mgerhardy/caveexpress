TEXTURE_DIR := contrib/assets/png-packed

.PHONY: gimp
gimp:
	@echo "Installing gimp patterns"
	$(Q)cp contrib/gimp/patterns/* ~/.gimp-*/patterns
	@echo "Installing gimp plugins"
	$(Q)cp contrib/gimp/plug-ins/* ~/.gimp-*/plug-ins

ifneq ($(PROGRAM_TEXTUREPACKER),)
.PHONY: texturepacker
texturepacker:
	@echo "Installing texturepacker exporters"
	$(Q)cp -r $(TEXTURE_DIR)/exporter/caveexpress /usr/share/texturepacker/exporters
endif
.PHONY: physicseditor
physicseditor:
	@echo "Installing physicseditor exporters"
	$(Q)cp -r $(TEXTURE_DIR)/exporter/caveexpress ~/.wine/drive_c/Program\ Files/PhysicsEditor/Exporters

textures: $(BASEDIR)/textures/complete.lua

$(BASEDIR)/textures/complete.lua: $(patsubst %.tps,%-big.lua,$(wildcard $(TEXTURE_DIR)/$(APPNAME)-*.tps)) $(patsubst %.tps,%-small.lua,$(wildcard $(TEXTURE_DIR)/$(APPNAME)-*.tps))
	@echo "===> SH [complete.lua]"
	$(Q)mkdir -p $(BASEDIR)/textures
	$(Q)echo "texturesbig = {" > $@
	$(Q)for i in $(TEXTURE_DIR)/$(APPNAME)-*-big.lua; do sed '1d' $$i | sed '$$d' >> $@; done
	$(Q)echo "}" >> $@
	$(Q)echo "texturessmall = {" >> $@
	$(Q)for i in $(TEXTURE_DIR)/$(APPNAME)-*-small.lua; do sed '1d' $$i | sed '$$d' >> $@; done
	$(Q)echo "}" >> $@

# --png-opt-level 7
TEXTURE_PACKER_OPTIONS += --data $@ --sheet $(patsubst $(TEXTURE_DIR)/%.lua,$(BASEDIR)/pics/%.png,$@) --size-constraints POT --texture-format png --max-width 2048 --max-height 2048 --quiet --scale-mode Smooth --dpi 72 --disable-rotation --pack-mode Best $< > /dev/null
ifneq ($(PROGRAM_TEXTUREPACKER),)
$(TEXTURE_DIR)/$(APPNAME)-u%-big.lua: $(TEXTURE_DIR)/$(APPNAME)-u%.tps
	@echo "===> TexturePacker [big] [$<]"
	$(Q)$(PROGRAM_TEXTUREPACKER) --scale 1.0 $(TEXTURE_PACKER_OPTIONS)
$(TEXTURE_DIR)/$(APPNAME)-npc_blowing-big.lua: $(TEXTURE_DIR)/$(APPNAME)-npc_blowing.tps
	@echo "===> TexturePacker [big] [$<]"
	$(Q)$(PROGRAM_TEXTUREPACKER) --scale 0.38 $(TEXTURE_PACKER_OPTIONS)
$(TEXTURE_DIR)/$(APPNAME)-%-big.lua: $(TEXTURE_DIR)/$(APPNAME)-%.tps
	@echo "===> TexturePacker [big] [$<]"
	$(Q)$(PROGRAM_TEXTUREPACKER) --scale 0.25 $(TEXTURE_PACKER_OPTIONS)

$(TEXTURE_DIR)/$(APPNAME)-u%-small.lua: $(TEXTURE_DIR)/$(APPNAME)-u%.tps
	@echo "===> TexturePacker [small] [$<]"
	$(Q)$(PROGRAM_TEXTUREPACKER) --scale 0.5 $(TEXTURE_PACKER_OPTIONS)
$(TEXTURE_DIR)/$(APPNAME)-npc_blowing-small.lua: $(TEXTURE_DIR)/$(APPNAME)-npc_blowing.tps
	@echo "===> TexturePacker [small] [$<]"
	$(Q)$(PROGRAM_TEXTUREPACKER) --scale 0.19 $(TEXTURE_PACKER_OPTIONS)
$(TEXTURE_DIR)/$(APPNAME)-%-small.lua: $(TEXTURE_DIR)/$(APPNAME)-%.tps
	@echo "===> TexturePacker [small] [$<]"
	$(Q)$(PROGRAM_TEXTUREPACKER) --scale 0.125 $(TEXTURE_PACKER_OPTIONS)
else
$(TEXTURE_DIR)/$(APPNAME)-%.lua:
endif
clean-textures:
	rm -f $(TEXTURE_DIR)/$(APPNAME)-*.lua

git:
	$(Q)git config diff.exif.textconv exiftool
	$(Q)git config core.eol lf
	$(Q)git config core.autocrlf input
	$(Q)git config branch.autosetuprebase always
	$(Q)git config branch.master.rebase true
	$(Q)git config branch.autosetuprebase always
	$(Q)git for-each-ref --sort=-committerdate refs/heads/|cut -d"	" -f2|cut -c12-|xargs -I {} git config branch.{}.rebase true
	$(Q)git config --global alias.upbase "rebase '@{u}'"
	$(Q)git config --global alias.wu "log --stat origin..@{0}"
	$(Q)git config --global push.default tracking
	$(Q)git config --global core.excludesfile '~/.gitignore'
	$(Q)echo "*~" >> ~/.gitignore
	$(Q)git config --global color.branch auto
	$(Q)git config --global color.diff auto
	$(Q)git config --global color.interactive auto
	$(Q)git config --global color.status auto
	$(Q)git config merge.keepOur.driver 'bash -c "exit 0"'
	$(Q)git config merge.keepOur.keepBackup false
	$(Q)git config merge.keepOur.recursive binary

pngcrush:
	$(Q)s="${SINCE}" ;\
	test -z "$$s" && s="$$(git log --grep=crush --pretty=format:%H|head -n 1)" ;\
	test -z "$$s" && s="$$(git rev-list --max-parents=0 HEAD)" ;\
	echo "Crushing images introduced since $$s" ;\
	git diff --diff-filter=AM --name-only "$$s" HEAD|grep '.png$$' \
	| xargs -I {} sh -c 'pngcrush -brute -rem alla {} /tmp/crush.png; mv -f /tmp/crush.png {}'

.PHONY: music-strip
music-strip:
	$(Q)for i in $(BASEDIR)/sounds/*.ogg; do printf '' | vorbiscomment -w $$i; done
 