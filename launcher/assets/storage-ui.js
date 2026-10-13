(function () {
  'use strict';
  const escapeHTML = value => String(value ?? '').replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  const defaults = [
    {id:'ruby',name:'Ruby'}, {id:'sapphire',name:'Sapphire'},
    {id:'firered',name:'FireRed'}, {id:'leafgreen',name:'LeafGreen'}, {id:'emerald',name:'Emerald'}
  ];
  const asset = 'assets/storage/';

  class StorageUI {
    constructor(container, hooks = {}) {
      this.container = container;
      this.hooks = hooks;
      this.catalog = hooks.catalog?.length ? hooks.catalog : defaults;
      this.registrations = [];
      this.sides = [this.newSide('firered'), this.newSide('leafgreen')];
      this.activeSide = 0;
      this.selection = null;
      this.target = null;
      this.busy = false;
      this.disposed = false;
      this.initializing = true;
      this.species = {};
      this.status = 'Choose a Pokémon, then choose an empty slot in the other box.';
      this.onClick = this.click.bind(this);
      this.onChange = this.change.bind(this);
      this.onFocus = this.focus.bind(this);
      container.classList.add('storage-ui');
      container.addEventListener('click', this.onClick);
      container.addEventListener('change', this.onChange);
      container.addEventListener('focusin', this.onFocus);
    }

    newSide(game) { return {game, box:0, slot:0, data:null, loading:false, error:null, sequence:0}; }
    title(id) { const g = this.catalog.find(g => g.id === id); return g?.name || g?.title || id; }
    registration(id) { return this.registrations.find(g => (g.id || g.game) === id); }
    registered(id) { const g = this.registration(id); return Boolean(g && (g.registered ?? g.path ?? g.saveExists)); }
    unwrap(data) { return data?.storage || data?.save || data?.data || data; }
    currentBox(side) { return side.data?.boxes?.[side.box]; }
    getMon(side, slot = side.slot) { return this.currentBox(side)?.mons?.[slot] || null; }

    async init() {
      this.initializing = true;
      this.render();
      const results = await Promise.allSettled([
        this.hooks.api('storage'),
        fetch(asset + 'species.json').then(r => r.ok ? r.json() : {})
      ]);
      if (this.disposed) return;
      if (results[1].status === 'fulfilled') this.species = results[1].value;
      if (results[0].status === 'rejected') {
        this.initializing = false;
        this.sides.forEach(s => s.error = results[0].reason?.message || 'Unable to read save files.');
        this.render();
        return;
      }
      const result = this.unwrap(results[0].value);
      this.registrations = result.games || [];
      const linked = this.catalog.filter(g => this.registered(g.id));
      const first = linked.find(g => g.id === 'firered') || linked[0];
      if (first) {
        this.sides[0].game = first.id;
        this.sides[1].game = (linked.find(g => g.id !== first.id) || first).id;
      }
      this.initializing = false;
      await Promise.all(this.sides.map((_, i) => this.load(i, true)));
    }

    async load(index, useCurrentBox = false) {
      const side = this.sides[index];
      const sequence = ++side.sequence;
      side.error = null;
      side.data = null;
      if (!this.registered(side.game)) { side.loading = false; this.render(); return; }
      side.loading = true;
      this.render();
      try {
        const result = this.unwrap(await this.hooks.api('storage?game=' + encodeURIComponent(side.game)));
        if (this.disposed || sequence !== side.sequence) return;
        if (!Array.isArray(result.boxes)) throw new Error('This save did not contain readable PC Boxes.');
        side.data = result;
        if (useCurrentBox) side.box = Math.max(0, Math.min(result.boxes.length - 1, Number(result.currentBox) || 0));
        side.box = Math.min(side.box, result.boxes.length - 1);
      } catch (error) {
        if (!this.disposed && sequence === side.sequence) side.error = error.message;
      } finally {
        if (!this.disposed && sequence === side.sequence) { side.loading = false; this.render(); }
      }
    }

    options(index) {
      const side = this.sides[index];
      return this.catalog.map(g => `<option value="${escapeHTML(g.id)}" ${g.id === side.game ? 'selected' : ''}>${escapeHTML(g.name || g.title || g.id)}${this.registered(g.id) ? '' : ' · Link save'}</option>`).join('');
    }

    pane(index) {
      const side = this.sides[index];
      const box = this.currentBox(side);
      let body;
      if (this.initializing || side.loading) {
        body = '<div class="pc-empty" role="status"><span class="pc-empty-ball"></span><h2>Reading save…</h2><p>Your boxes will appear here.</p></div>';
      } else if (side.error) {
        body = `<div class="pc-empty pc-error" role="alert"><h2>Unable to open this save</h2><p>${escapeHTML(side.error)}</p><button data-pc-action="reload" data-side="${index}">Try again</button><button data-pc-action="import" data-side="${index}">Choose another save</button></div>`;
      } else if (!side.data) {
        body = `<div class="pc-empty"><span class="pc-empty-ball"></span><h2>${escapeHTML(this.title(side.game))} PC</h2><p>Link your ${escapeHTML(this.title(side.game))} .sav file to see its Pokémon.</p><button class="pc-primary" data-pc-action="import" data-side="${index}">Choose save file</button><form class="pc-path" data-side="${index}"><label for="pc-path-${index}">Or enter the save file path</label><div><input id="pc-path-${index}" type="text" aria-label="${escapeHTML(this.title(side.game))} save path" placeholder="C:\\Games\\${escapeHTML(side.game)}.sav" autocomplete="off"><button type="button" data-pc-action="link-path" data-side="${index}">Link</button></div></form><small>The original save file stays in its current location.</small></div>`;
      } else {
        const wallpaper = Math.min(15, Math.max(0, Number(box?.wallpaper) || 0));
        const count = (box?.mons || []).filter(Boolean).length;
        body = `<div class="pc-box" style="background-image:url('${asset}wallpapers/${wallpaper}.png')"><div class="pc-box-title"><button data-pc-action="prev-box" data-side="${index}" aria-label="Previous ${index ? 'destination' : 'source'} box">◀</button><select data-pc-box="${index}" aria-label="${index ? 'Destination' : 'Source'} box">${side.data.boxes.map((b, i) => `<option value="${i}" ${i === side.box ? 'selected' : ''}>${escapeHTML(b.name || 'BOX ' + (i + 1))}</option>`).join('')}</select><button data-pc-action="next-box" data-side="${index}" aria-label="Next ${index ? 'destination' : 'source'} box">▶</button></div><div class="pc-slots" role="grid" aria-label="${escapeHTML(this.title(side.game))}, ${escapeHTML(box?.name || 'BOX ' + (side.box + 1))}">${Array.from({length:30}, (_, slot) => this.slot(index, slot)).join('')}</div><span class="pc-count">${count}/30</span></div>`;
      }
      const trainer = typeof side.data?.trainer === 'object' ? side.data.trainer.name : side.data?.trainer;
      return `<section class="pc-pane ${index === this.activeSide ? 'pc-active-pane' : ''}" aria-label="${index ? 'Destination' : 'Source'} Pokémon box"><div class="pc-pane-bar"><span class="pc-pane-label">${index ? 'TO' : 'FROM'}</span><select data-pc-game="${index}" aria-label="${index ? 'Destination' : 'Source'} game">${this.options(index)}</select><span class="pc-trainer" title="Trainer">${escapeHTML(trainer || '')}${side.data?.readOnly ? ' · READ ONLY' : ''}</span><button data-pc-action="import" data-side="${index}" title="Link a save file for ${escapeHTML(this.title(side.game))}" aria-label="Link ${escapeHTML(this.title(side.game))} save">Link save</button></div>${body}</section>`;
    }

    slot(index, slot) {
      const side = this.sides[index];
      const mon = this.getMon(side, slot);
      const same = point => point && point.game === side.game && point.box === side.box && point.slot === slot;
      const source = same(this.selection), target = same(this.target);
      const active = index === this.activeSide && slot === side.slot;
      const name = mon ? (mon.nickname || mon.name || this.species[mon.species]?.name || 'Pokémon') : 'Empty';
      const valid = !mon || mon.valid !== false;
      return `<button class="pc-slot ${active ? 'pc-cursor' : ''} ${source ? 'pc-picked' : ''} ${target ? 'pc-target' : ''} ${!valid ? 'pc-invalid' : ''}" data-pc-slot="${slot}" data-side="${index}" role="gridcell" tabindex="${active ? 0 : -1}" aria-selected="${Boolean(source || target)}" aria-label="Slot ${slot + 1}: ${escapeHTML(name)}${mon?.level ? ', level ' + mon.level : ''}${!valid ? ', damaged data' : ''}">${mon ? `<img class="pc-icon" src="${asset}icons/${Number(mon.species)}.png" alt="" draggable="false">${mon.shiny ? '<span class="pc-shiny" title="Shiny">✦</span>' : ''}${!valid ? '<span class="pc-invalid-mark">!</span>' : ''}` : ''}<img class="pc-hand" src="${asset}cursor.png" alt="" draggable="false">${source ? '<span class="pc-slot-tag">PICKED</span>' : target ? '<span class="pc-slot-tag">HERE</span>' : ''}</button>`;
    }

    summary() {
      const mon = this.selection?.mon;
      const focusMon = this.getMon(this.sides[this.activeSide]);
      const shown = mon || focusMon;
      const name = shown?.nickname || shown?.name || this.species[shown?.species]?.name || '';
      const speciesName = shown?.name || this.species[shown?.species]?.name || '';
      const genderValue = String(shown?.gender || '').toLowerCase();
      const gender = genderValue === 'male' || genderValue === 'm' ? '♂' : genderValue === 'female' || genderValue === 'f' ? '♀' : '';
      const moves = (shown?.moves || []).filter(m => m.id || m.name).map(m => `${m.name || 'MOVE ' + m.id}${m.pp != null ? ' ' + m.pp + 'PP' : ''}`).join('  ·  ');
      const targetSide = this.sides[1];
      const targetBox = this.target && targetSide.data?.boxes?.[this.target.box];
      const targetText = this.target ? `${this.title(this.target.game)} · ${targetBox?.name || 'BOX ' + (this.target.box + 1)} · Slot ${this.target.slot + 1}` : 'Choose an empty slot in the right box.';
      const ready = this.canMove();
      return `<div class="pc-summary" aria-live="polite"><div class="pc-portrait">${shown ? `<img src="${asset}front/${Number(shown.species)}.png" alt="${escapeHTML(speciesName || name)}">` : '<span>?</span>'}</div><div class="pc-mon-info">${shown ? `<h2>${escapeHTML(name)} <span class="pc-gender">${escapeHTML(gender)}</span>${shown.shiny ? '<span class="pc-shiny-name">✦</span>' : ''}<span class="pc-level">Lv.${escapeHTML(shown.level ?? '—')}</span></h2><p>${escapeHTML(speciesName)}${shown.nature ? ' · ' + escapeHTML(shown.nature) : ''}${shown.trainerName ? ' · OT ' + escapeHTML(shown.trainerName) : ''}</p><p>Held item: ${escapeHTML(shown.heldItem || 'None')}</p><p class="pc-moves">${escapeHTML(moves || 'No moves')}</p>${shown.valid === false ? `<p class="pc-mon-warning">${escapeHTML(shown.error || 'Damaged Pokémon data cannot be moved.')}</p>` : ''}` : '<h2>Which Pokémon will you move?</h2><p>Select a Pokémon in the left box.</p><p>All five adventures can share Pokémon here.</p>'}</div><div class="pc-transfer"><p class="pc-transfer-caption">${this.selection ? 'DESTINATION' : 'PC STORAGE'}</p><p>${escapeHTML(this.selection ? targetText : 'Your Pokémon. Your game saves.')}</p><div><button class="pc-primary" data-pc-action="move" ${ready ? '' : 'disabled'}>${this.busy ? 'Saving…' : 'Move Pokémon'}</button><button data-pc-action="clear" ${this.selection || this.target ? '' : 'disabled'}>Cancel</button></div></div></div>`;
    }

    render(focusSlot = false) {
      if (this.disposed) return;
      this.container.setAttribute('aria-busy', String(this.busy || this.initializing));
      this.container.innerHTML = `<div class="pc-panes">${this.pane(0)}${this.pane(1)}</div>${this.summary()}<div class="pc-status" role="status"><span>${escapeHTML(this.status)}</span><span>Changes save directly to your game files. Automatic backups.</span></div>`;
      if (this.busy) this.container.querySelectorAll('button,select,input').forEach(el => el.disabled = true);
      this.container.querySelectorAll('form').forEach(form => form.addEventListener('submit', event => { event.preventDefault(); this.linkPath(Number(form.dataset.side)); }));
      this.container.querySelectorAll('img').forEach(img => img.addEventListener('error', () => { img.hidden = true; }));
      if (focusSlot) this.slotElement(this.activeSide, this.sides[this.activeSide].slot)?.focus({preventScroll:true});
    }

    slotElement(side, slot) { return this.container.querySelector(`[data-pc-slot="${slot}"][data-side="${side}"]`); }

    focus(event) {
      const slot = event.target.closest('[data-pc-slot]');
      if (!slot) return;
      this.activeSide = Number(slot.dataset.side);
      this.sides[this.activeSide].slot = Number(slot.dataset.pcSlot);
      this.container.querySelectorAll('.pc-cursor').forEach(el => el.classList.remove('pc-cursor'));
      slot.classList.add('pc-cursor');
      this.container.querySelectorAll('.pc-pane').forEach((el, i) => el.classList.toggle('pc-active-pane', i === this.activeSide));
    }

    click(event) {
      if (this.busy) return;
      const slot = event.target.closest('[data-pc-slot]');
      if (slot) { this.pick(Number(slot.dataset.side), Number(slot.dataset.pcSlot)); return; }
      const button = event.target.closest('[data-pc-action]');
      if (!button || button.disabled) return;
      const index = Number(button.dataset.side || 0);
      const action = button.dataset.pcAction;
      if (action === 'import') this.browse(index);
      else if (action === 'link-path') this.linkPath(index);
      else if (action === 'reload') this.registrations.length ? this.load(index) : this.init();
      else if (action === 'prev-box' || action === 'next-box') this.changeBox(index, this.sides[index].box + (action === 'next-box' ? 1 : -1));
      else if (action === 'clear') this.clear();
      else if (action === 'move') this.reviewMove();
    }

    change(event) {
      if (this.busy) return;
      if (event.target.matches('[data-pc-game]')) {
        const index = Number(event.target.dataset.pcGame);
        this.sides[index].game = event.target.value;
        this.sides[index].box = 0;
        this.activeSide = index;
        if (!index) this.selection = null;
        this.target = null;
        this.load(index, true);
      } else if (event.target.matches('[data-pc-box]')) this.changeBox(Number(event.target.dataset.pcBox), Number(event.target.value));
    }

    changeBox(index, box) {
      const side = this.sides[index];
      if (!side.data || this.busy) return;
      side.box = (box + side.data.boxes.length) % side.data.boxes.length;
      this.activeSide = index;
      if (index === 1) this.target = null;
      this.render();
    }

    pick(index, slot) {
      const side = this.sides[index];
      if (!side.data || this.busy) return;
      this.activeSide = index;
      side.slot = slot;
      const mon = this.getMon(side, slot);
      if (index === 0) {
        if (mon) {
          this.selection = {game:side.game, box:side.box, slot, mon, revision:side.data.revision};
          this.target = null;
          this.status = mon.valid === false ? 'This Pokémon has damaged data and cannot be moved.' : `Choose an empty destination for ${mon.nickname || mon.name || 'this Pokémon'}.`;
        } else {
          this.selection = null;
          this.target = null;
          this.status = 'This slot is empty. Choose a Pokémon to move.';
        }
      } else if (mon) {
        this.target = null;
        this.status = 'This destination is occupied. Choose an empty slot.';
      } else if (!this.selection) {
        this.status = 'First select a Pokémon in the left box.';
      } else {
        const already = this.target && this.target.game === side.game && this.target.box === side.box && this.target.slot === slot;
        this.target = {game:side.game, box:side.box, slot, revision:side.data.revision};
        this.status = 'Destination selected. Choose Move Pokémon to review.';
        if (already && this.canMove()) { this.reviewMove(); return; }
      }
      this.render(true);
    }

    canMove() {
      if (this.busy || !this.selection || !this.target || this.selection.mon.valid === false) return false;
      if (this.sides.some(s => s.loading || s.error || s.data?.readOnly)) return false;
      if (this.sides[0].data?.revision !== this.selection.revision) return false;
      const target = this.target;
      const data = this.sides[1].data;
      if (!data || data.revision !== target.revision || data.boxes[target.box]?.mons?.[target.slot]) return false;
      return !(this.selection.game === target.game && this.selection.box === target.box && this.selection.slot === target.slot);
    }

    clear() {
      this.selection = null;
      this.target = null;
      this.status = 'Choose a Pokémon, then choose an empty slot in the other box.';
      this.render();
    }

    async browse(index) {
      if (this.busy) return;
      this.busy = true;
      this.render();
      try {
        const result = await this.hooks.api('browse', {kind:'save'});
        if (result.path && !this.disposed) await this.importSave(index, result.path);
      } catch (error) { if (!this.disposed) this.hooks.showError?.(error); }
      finally { if (!this.disposed) { this.busy = false; this.render(); } }
    }

    async linkPath(index) {
      const input = this.container.querySelector('#pc-path-' + index);
      const path = input?.value.trim();
      if (!path) { input?.focus(); return; }
      this.busy = true;
      this.render();
      try { await this.importSave(index, path); }
      catch (error) { if (!this.disposed) this.hooks.showError?.(error); }
      finally { if (!this.disposed) { this.busy = false; this.render(); } }
    }

    async importSave(index, path) {
      const game = this.sides[index].game;
      await this.hooks.api('storage/import', {game, path});
      if (this.disposed) return;
      const state = this.unwrap(await this.hooks.api('storage'));
      this.registrations = state.games || this.registrations;
      this.selection = null;
      this.target = null;
      await Promise.all(this.sides.map((s, i) => s.game === game ? this.load(i, true) : Promise.resolve()));
      this.hooks.toast?.(`${this.title(game)} save linked.`);
    }

    reviewMove() {
      if (!this.canMove()) return;
      const source = {...this.selection}, target = {...this.target};
      const name = source.mon.nickname || source.mon.name || 'Pokémon';
      const sourceName = this.sides[0].data?.boxes[source.box]?.name || 'BOX ' + (source.box + 1);
      const targetName = this.sides[1].data?.boxes[target.box]?.name || 'BOX ' + (target.box + 1);
      this.hooks.dialog(`Move ${name}?`, `${this.title(source.game)} · ${sourceName} · Slot ${source.slot + 1}\n→ ${this.title(target.game)} · ${targetName} · Slot ${target.slot + 1}\n\nYour Pokémon will leave the source slot and appear in the destination. Automatic backups are created before saving.`, 'Move Pokémon', () => this.performMove(source, target));
    }

    async performMove(source, target) {
      if (this.busy || this.disposed) return;
      this.busy = true;
      this.hooks.closeDialog?.();
      this.status = 'Saving your Pokémon…';
      this.render();
      try {
        await this.hooks.api('storage/move', {
          sourceGame:source.game, sourceBox:source.box, sourceSlot:source.slot,
          targetGame:target.game, targetBox:target.box, targetSlot:target.slot,
          sourceRevision:source.revision, targetRevision:target.revision
        });
        if (this.disposed) return;
        this.selection = null;
        this.target = null;
        await Promise.all([this.load(0), this.load(1)]);
        this.status = source.game === target.game ? 'Saved directly to your game save; automatic backup created.' : 'Saved directly to both game saves; automatic backups.';
        this.hooks.toast?.('Pokémon moved. ' + this.status);
      } catch (error) {
        if (this.disposed) return;
        this.selection = null;
        this.target = null;
        this.status = 'Unable to confirm the move. Check the refreshed boxes before trying again.';
        await Promise.all([this.load(0), this.load(1)]);
        this.hooks.showError?.(error);
      } finally {
        if (!this.disposed) { this.busy = false; this.render(); }
      }
    }

    key(key) {
      if (this.disposed || this.busy) return false;
      const focused = document.activeElement;
      if (focused && this.container.contains(focused) && focused.matches('input,select,textarea')) return false;
      if (key === 'Escape' || key === 'Backspace') {
        if (this.selection || this.target) this.clear(); else this.hooks.back?.();
        return true;
      }
      if (key === 'Enter' || key === ' ' || key === 'a') {
        if (focused && this.container.contains(focused) && focused.matches('button:not([data-pc-slot])')) focused.click();
        else if (!this.sides[this.activeSide].data && !this.sides[this.activeSide].loading && !this.initializing) this.browse(this.activeSide);
        else this.pick(this.activeSide, this.sides[this.activeSide].slot);
        return true;
      }
      if (key === 'PageUp' || key === 'PageDown') {
        this.changeBox(this.activeSide, this.sides[this.activeSide].box + (key === 'PageDown' ? 1 : -1));
        return true;
      }
      if (!['ArrowLeft','ArrowRight','ArrowUp','ArrowDown'].includes(key)) return false;
      const current = this.sides[this.activeSide];
      let row = Math.floor(current.slot / 6), col = current.slot % 6;
      if (key === 'ArrowLeft') { if (col === 0 && this.activeSide === 1) {this.activeSide = 0; col = 5;} else col = (col + 5) % 6; }
      if (key === 'ArrowRight') { if (col === 5 && this.activeSide === 0) {this.activeSide = 1; col = 0;} else col = (col + 1) % 6; }
      if (key === 'ArrowUp') row = (row + 4) % 5;
      if (key === 'ArrowDown') row = (row + 1) % 5;
      this.sides[this.activeSide].slot = row * 6 + col;
      this.render(true);
      return true;
    }

    dispose() {
      this.disposed = true;
      this.sides.forEach(side => side.sequence++);
      this.container.removeEventListener('click', this.onClick);
      this.container.removeEventListener('change', this.onChange);
      this.container.removeEventListener('focusin', this.onFocus);
    }
  }
  window.StorageUI = StorageUI;
})();
