'use strict';
const $ = (selector, parent = document) => parent.querySelector(selector);
const $$ = (selector, parent = document) => [...parent.querySelectorAll(selector)];
const stage = $('#stage'), screen = $('#screen'), modalRoot = $('#modal-root');
const catalog = [
  {id:'ruby',name:'Ruby',title:'Pokémon Ruby Version',color:'#851e28',species:'groudon',region:'HOENN',disabled:true},
  {id:'sapphire',name:'Sapphire',title:'Pokémon Sapphire Version',color:'#173d8a',species:'kyogre',region:'HOENN',disabled:true},
  {id:'firered',name:'FireRed',title:'Pokémon FireRed Version',color:'#a84321',species:'charizard',region:'KANTO'},
  {id:'leafgreen',name:'LeafGreen',title:'Pokémon LeafGreen Version',color:'#426b28',species:'venusaur',region:'KANTO'},
  {id:'emerald',name:'Emerald',title:'Pokémon Emerald Version',color:'#196452',species:'rayquaza',region:'HOENN'}
];
let state = {games:[],settings:{},defaults:{},running:null};
let route = 'notice', selected = 0, gameIndex = 2, optionsTab = null, pending = false;
let dialogAction = null, restoreFocus = null, toastTimer, statusTimer, apiConnected = false;
let launcherAudio, previewVolume=null, launchingGame=false, carousel, storageUI, titleEffects, noticeTimer, sceneTransitions;
let navigationPending=false;
let navigationGeneration=0, stateGeneration=0, gameGeneration=0, gameReturnNotified=true, nativeReturnTask=null;
const titleSeed = crypto.getRandomValues(new Uint32Array(1))[0];
let titleTheme = titleSeed % 2 ? 'fire' : 'leaf';
// Returning from a game keeps this opening's theme; the next opening alternates.
try { const previous = localStorage.getItem('pokemon-title-theme'); if(previous==='fire'||previous==='leaf')titleTheme=previous==='fire'?'leaf':'fire'; localStorage.setItem('pokemon-title-theme',titleTheme); } catch {}
const esc = value => String(value ?? '').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
const ball = '<img class="ball" src="assets/ui/pokeball.svg" alt="">';
const game = () => catalog[gameIndex];
const installed = id => state.games.find(g=>g.id===id) || {available:false,saveExists:false};
function resize(){stage.style.transform=`scale(${Math.min(innerWidth/1280,innerHeight/720)})`;}
addEventListener('resize',resize);resize();
function syncAudio(){launcherAudio?.setState({route,playing:Boolean(state.running)||launchingGame,masterVolume:previewVolume??state.settings.volume??8});}
function tone(){launcherAudio?.select();}
function toast(message){$('#toast').textContent=message;$('#toast').classList.add('show');clearTimeout(toastTimer);toastTimer=setTimeout(()=>$('#toast').classList.remove('show'),4800);}
async function api(path,body){
  const response=await fetch(`/api/${path}`,body===undefined?{cache:'no-store'}:{method:'POST',headers:{'Content-Type':'application/json','X-Launcher-Token':state.token||state.csrfToken||''},body:JSON.stringify(body)});
  const data=await response.json();if(!response.ok||data.ok===false)throw new Error(data.error||'The launcher could not complete that action.');return data;
}
async function refresh(initial=false){
  const generation=stateGeneration;
  try{const old=state.running,nextState=await api('state');if(generation!==stateGeneration)return;state=nextState;apiConnected=true;syncAudio();
    if(old&&!state.running){restoreLauncherAfterGame(nextState);return;}
    if(!old&&state.running)gameReturnNotified=false;
    if(initial||(!modalRoot.firstChild&&Boolean(old)!==Boolean(state.running))) render();
  }catch(error){if(generation!==stateGeneration)return;apiConnected=false;if(initial){render();toast('Launcher service is unavailable. Open Launch PokeRecomp.cmd to start.');}}
}
function announceGameReturn(){
  if(gameReturnNotified)return;
  gameReturnNotified=true;
  if(state.lastExit?.exitCode)showError(new Error(state.lastExit.error||'The game could not open this session. Check that the save is a valid FireRed cartridge save.'));
  else toast('Game closed. Welcome back.');
}
function restoreLauncherAfterGame(nextState=null,notify=true,rebuild=true){
  ++navigationGeneration;++stateGeneration;++gameGeneration;
  pending=false;launchingGame=false;navigationPending=false;lastPadAction='';lastPadTime=0;
  if(nextState)state=nextState;else state={...state,running:null};
  sceneTransitions?.reset();
  syncAudio();launcherAudio?.endTransition();
  // Keep any error or confirmation dialog intact; only the underlying screen is rebuilt.
  if(rebuild)render();
  if(notify)announceGameReturn();
}
function nativeGameReturned(){
  if(nativeReturnTask||(!state.running&&!launchingGame&&gameReturnNotified))return;
  restoreLauncherAfterGame(null,false,false);
  const generation=stateGeneration;
  nativeReturnTask=(async()=>{
    try{
      const nextState=await api('state');
      if(generation!==stateGeneration)return;
      state=nextState;apiConnected=true;syncAudio();
      if(!state.running){announceGameReturn();if(!navigationPending)render();}
    }catch(error){if(generation===stateGeneration){apiConnected=false;render();announceGameReturn();}}
    finally{nativeReturnTask=null;}
  })();
}
window.addEventListener('launcher-game-return',nativeGameReturned);
function logo(className){return `<img class="logo ${className}" src="assets/logo.png" alt="Pokémon Recompiled" data-logo><span class="brand-fallback ${className}" hidden>Pokémon Recompiled</span><span class="logo-sheen ${className}" aria-hidden="true"></span>`;}
function header(title){return `<header class="topbar"><h1>${ball}${title}</h1></header>`;}
function footer(confirm='Confirm'){return `<footer class="footer"><button class="hint" data-action="confirm" aria-label="${confirm}"><span class="key">A</span>${confirm}</button><button class="hint" data-action="back" aria-label="Back"><span class="key back">B</span>Back</button></footer>`;}
function menu(items,className=''){return `<nav class="menu ${className}" aria-label="${route} menu">${items.map(([label,action,icon],i)=>`<button class="menu-button ${i===selected?'selected':''}" data-action="${action}" data-menu="${i}">${icon?ball:''}<span>${label}</span></button>`).join('')}</nav>`;}
function changeScreen(update,nextRoute=route){if(navigationPending||sceneTransitions?.busy)return;navigationPending=true;const generation=++navigationGeneration;return transitionsReady.then(()=>{if(generation!==navigationGeneration)return;const audioFade=launcherAudio?.beginTransition(nextRoute);const swap=async()=>{await audioFade;if(generation!==navigationGeneration)return;await update();if(generation===navigationGeneration)launcherAudio?.endTransition();};return sceneTransitions?sceneTransitions.run(swap):swap();}).finally(()=>{if(generation===navigationGeneration){launcherAudio?.endTransition();navigationPending=false;}});}
function go(next){if(navigationPending||sceneTransitions?.busy)return;tone();return changeScreen(()=>{route=next;selected=0;optionsTab=null;return render();},next);}
function render(){
  if(route!=='options'||optionsTab!=='audio')previewVolume=null;
  syncAudio();
  carousel?.dispose();carousel=null;
  storageUI?.dispose();storageUI=null;
  titleEffects?.dispose();titleEffects=null;
  clearTimeout(noticeTimer);
  const background=route==='storage'?'boxes':route;
  $('#backdrop').style.backgroundImage=['title','notice'].includes(route)?'none':`url('assets/backgrounds/${background}.png')`;
  stage.dataset.screen=route;stage.dataset.titleTheme=titleTheme;screen.classList.remove('enter');void screen.offsetWidth;
  if(route==='notice'){
    screen.innerHTML='<section class="opening-notice" aria-labelledby="notice-heading"><h1 id="notice-heading">POKÉMON RECOMPILED</h1><p>Pokémon and its characters, artwork, names, music, and game content belong to Nintendo, GAME FREAK, Creatures Inc., and The Pokémon Company.</p><p>An unofficial fan project.<br>Not affiliated with or endorsed by those companies.</p><p>No games or ROM downloads are provided through this launcher.<br>Use your own game files and saves.</p><p class="notice-copyright">© Nintendo / Creatures Inc. / GAME FREAK inc.<br>Pokémon is a trademark of Nintendo.</p></section><button class="notice-continue" data-action="notice-continue">CONTINUE <span>ENTER / A</span></button>';
    noticeTimer=setTimeout(()=>{if(route==='notice')go('title');},8000);
  }else if(route==='title'){
    screen.innerHTML=`<div class="title-field" aria-hidden="true"></div><div id="title-effects" aria-hidden="true"></div><img class="title-art title-charizard" src="assets/characters/charizard-boxart-official.png" alt="Charizard"><img class="title-art title-venusaur" src="assets/characters/venusaur-boxart-official.png" alt="Venusaur">${logo('title-logo')}<button class="press-start" data-action="start">PRESS START</button><div class="title-copyright">© Nintendo / Creatures Inc. / GAME FREAK inc.<br>Pokémon Recompiled · Unofficial fan project</div>`;
  }else if(route==='main'){
    screen.innerHTML=`${logo('main-logo')}<div class="main-edition">GEN 3</div>${menu([['Game Select','games',1],['PC Boxes','boxes',1],['Mystery Gifts','gifts',1],['Options','options',1],['Quit','quit',1]])}<button class="credits-link" data-action="about">About Pokémon Recompiled</button>`;
  }else if(route==='games'){
    screen.innerHTML=`${header('Game Select')}<button class="carousel-arrow prev" data-action="previous-game" aria-label="Previous game"><img src="assets/ui/cursor.svg" alt=""></button><div id="game-carousel" aria-label="3D game case carousel"></div><div class="game-tabs" aria-label="Game library">${catalog.map((g,i)=>`<button class="game-tab ${i===gameIndex?'selected':''} ${g.disabled?'disabled':''}" data-game="${i}" aria-label="${g.title}${g.disabled?', not available yet':installed(g.id).available?'':', import ROM'}" aria-pressed="${i===gameIndex}">${g.name}${!g.disabled&&!installed(g.id).available?'<span>IMPORT ROM</span>':''}</button>`).join('')}</div><button class="carousel-arrow next" data-action="next-game" aria-label="Next game"><img src="assets/ui/cursor.svg" alt=""></button><div class="game-caption ${game().disabled?'disabled':''}"><h2 class="game-title">${game().title}</h2><p>${game().region} REGION</p></div>${game().disabled?'':'<div class="library-action"><button class="small-action" data-action="import-game">Import ROM</button><button class="small-action" data-action="import-save">Import save</button></div>'}${footer(game().disabled?undefined:installed(game().id).available?'Play':'Import ROM')}`;
  }else if(route==='boxes'){
    screen.innerHTML=`${header('PC Boxes')}${menu([['View Boxes','box-view'],['Organize Pokémon','box-organize'],['Move Pokémon','box-move'],['See Details','box-details']],'boxes-menu')}${footer()}`;
  }else if(route==='storage'){
    screen.innerHTML=`${header('PC Boxes')}<div id="storage-root"></div>${footer('Select')}`;
  }else if(route==='gifts'){
    screen.innerHTML=`${header('Mystery Gifts')}${menu([['Receive Gift','gift-receive'],['Check Gifts','gift-check'],['Gift Information','gift-info']],'gifts-menu')}<div class="screen-message">${esc(state.giftStatus?.message||'Explore Mystery Gift with your FireRed save.')}</div>${footer()}`;
  }else if(route==='options'){
    screen.innerHTML=`${header('Options')}${optionsTab?settingsPanel():menu([['Game Settings','settings-game'],['Display Settings','settings-display'],['Audio Settings','settings-audio'],['Control Settings','settings-controls'],['Language','settings-language'],['Reset to Default','reset-settings']],'compact')}${footer(optionsTab&&['game','display','audio'].includes(optionsTab)?'Save':'Confirm')}`;
    if(optionsTab)bindSettings();
  }
  if(route==='gifts')screen.insertAdjacentHTML('afterbegin','<img class="scene-character gift-mew" src="assets/characters/mew-3d.png" alt="">');
  if(state.running&&!['title','notice'].includes(route))screen.insertAdjacentHTML('beforeend',`<div class="running-banner" role="status">${['boxes','organize','move','details'].includes(state.running.mode)?'PC Boxes: press Z (B), then choose NO to save and return.':esc(state.running.title||state.running.game||'Game')+' is running.'}</div>`);
  bind();
  const effectsReady=route==='title'?initTitleEffects():Promise.resolve();
  if(route==='games')initCarousel();
  if(route==='storage')initStorage();
  return Promise.all([effectsReady,...$$('img',screen).map(img=>img.decode?.().catch(()=>{}))]);
}
async function initTitleEffects(){const container=$('#title-effects');try{const {mountTitleEffects}=await import('./assets/title-effects.js');if(route!=='title'||!container?.isConnected)return;titleEffects=mountTitleEffects(container,{theme:titleTheme,seed:titleSeed});await titleEffects.ready;}catch(error){console.error('Title effects unavailable',error);}}
async function initStorage(){try{if(!window.StorageUI)await import('./assets/storage-ui.js');if(!$('#storage-styles')){const link=document.createElement('link');link.id='storage-styles';link.rel='stylesheet';link.href='assets/storage-ui.css';document.head.appendChild(link);}if(route!=='storage')return;storageUI=new window.StorageUI($('#storage-root'),{api,toast,dialog,closeDialog,showError,back,catalog});await storageUI.init();}catch(error){showError(error);}}
function initCarousel(){if(carousel||!window.GameCarousel||!$('#game-carousel'))return;try{carousel=new window.GameCarousel($('#game-carousel'),catalog,{index:gameIndex,onSelect:index=>{if(index===gameIndex)launchGame();else selectGame(index);}});}catch(error){console.error('3D carousel unavailable',error);$('#game-carousel').innerHTML=catalog.map(g=>`<img class="cover-fallback" src="assets/covers/${g.id}.png" alt="${g.title}">`).join('');toast('3D graphics are unavailable. You can still select a game below.');}}
window.addEventListener('carousel-ready',()=>{if(route==='games')initCarousel();});
function selectGame(index){gameIndex=(index+catalog.length)%catalog.length;carousel?.setIndex(gameIndex);$$('[data-game]',screen).forEach(b=>{const active=Number(b.dataset.game)===gameIndex;b.classList.toggle('selected',active);b.setAttribute('aria-pressed',String(active));});$('.game-title',screen).textContent=game().title;$('.game-caption p',screen).textContent=`${game().region} REGION · ${installed(game().id).available?(installed(game().id).saveExists?'SAVE DATA FOUND':'A NEW ADVENTURE AWAITS'):'SELECT TO IMPORT YOUR ROM'}`;const confirm=$('[data-action="confirm"]',screen);const label=installed(game().id).available?'Play':'Add game';confirm.innerHTML=`<span class="key">A</span>${label}`;confirm.setAttribute('aria-label',label);tone();}
function bind(){
  $$('img',screen).forEach(img=>img.addEventListener('error',()=>{img.style.visibility='hidden';if(img.hasAttribute('data-logo'))img.nextElementSibling.hidden=false;}));
  $$('[data-action]',screen).forEach(button=>button.addEventListener('click',()=>action(button.dataset.action)));
  $$('[data-menu]',screen).forEach(button=>{
    button.addEventListener('pointerenter',()=>selectMenu(Number(button.dataset.menu)));
    button.addEventListener('focus',()=>selectMenu(Number(button.dataset.menu)));
  });
  $$('[data-game]',screen).forEach(button=>{
    button.addEventListener('click',()=>{const index=Number(button.dataset.game);if(index===gameIndex)launchGame();else selectGame(index);});
  });
}
function selectMenu(index,sound=true){const buttons=$$('[data-menu]',screen);if(!buttons.length)return;const next=(index+buttons.length)%buttons.length,changed=next!==selected;selected=next;buttons.forEach((b,i)=>b.classList.toggle('selected',i===selected));if(sound&&changed)tone();}
function back(){if(modalRoot.firstChild){tone();closeDialog();return;}if(route==='notice'){go('title');return;}if(optionsTab){tone();changeScreen(()=>{optionsTab=null;selected=0;return render();});return;}if(route==='storage'){go('boxes');return;}if(route==='main'){go('title');return;}if(route!=='title')go('main');}
function confirm(){tone();if(route==='notice')go('title');else if(route==='title')go('main');else if(route==='games')launchGame();else if(route==='storage')storageUI?.key('Enter');else if(optionsTab){if(['game','audio','display'].includes(optionsTab))saveSettings();else if(optionsTab==='controls'){const f=document.activeElement;if(f?.matches?.('.bind-cell,#reset-binds,[data-action="back"]'))f.click();else $('.bind-cell',screen)?.focus();}else back();}else $(`[data-menu="${selected}"]`,screen)?.click();}
async function action(name){
  if(pending||navigationPending||sceneTransitions?.busy)return;
  if(name==='notice-continue'){go('title');return;}
  if(['main','games','boxes','gifts','options'].includes(name)){go(name);return;}
  if(name==='start'){go('main');return;}if(name==='back'){back();return;}if(name==='confirm'){confirm();return;}
  if(name==='previous-game'||name==='next-game'){selectGame(gameIndex+(name==='next-game'?1:-1));return;}
  if(name.startsWith('settings-')){changeScreen(()=>{optionsTab=name.slice(9);return render();});return;}
  if(name.startsWith('box-')){
    go('storage');
    return;
  }
  if(name==='import-game'){romDialog();return;}if(name==='import-save'){importDialog('save');return;}
  if(name==='reset-settings'){dialog('Restore default settings?','This resets display, audio, fast-forward and control settings. Your adventure and save data are kept.','Reset settings',async()=>{try{const result=await api('settings',{settings:state.defaults});state.settings=result.settings;closeDialog();toast('Default settings restored.');render();}catch(e){showError(e);}});return;}
  if(name==='gift-receive'){dialog('Receive a Mystery Gift','The original FireRed Mystery Gift menu is available with a saved adventure. Internet gift distribution is not configured for this project.','Open game menu',()=>launch('firered','gifts'));return;}
  if(name==='gift-check'){dialog('Check Gifts','Wonder Cards and Wonder News are stored in your FireRed save. Open the original Mystery Gift menu to view them.','Open game menu',()=>launch('firered','gifts'));return;}
  if(name==='gift-info'){dialog('Mystery Gift','Use the original FireRed Mystery Gift screen to check Wonder Cards and Wonder News in your save.\n\nThis launcher has no configured internet gift service. It will never claim a gift was received unless the game actually receives it.','Got it',closeDialog,false);return;}
  if(name==='about'){dialog('Pokémon Recompiled','Five classic adventures, one home.\n\nPC Boxes uses the original FireRed/LeafGreen graphics and Gen 3 save formats. Pokémon can move between all five games without starting a game.\n\nPokémon belongs to Nintendo, GAME FREAK, Creatures Inc., and The Pokémon Company. This is an unofficial fan project. No games or ROM downloads are provided through the launcher.','Back',closeDialog,false);return;}
  if(name==='quit'){if(state.running){toast('Close the running game before quitting the launcher.');return;}try{await api('quit',{});clearInterval(statusTimer);launcherAudio?.dispose();screen.innerHTML='<div class="closed"><h1>See you next adventure.</h1><p>The launcher is closed. You can close this window.</p></div>';if(window.chrome?.webview)window.chrome.webview.postMessage({type:'quit'});else setTimeout(()=>window.close(),200);}catch(e){showError(e);}return;}
}
async function launchGame(){if(game().disabled)return;if(installed(game().id).available)await launch(game().id,'game');else romDialog();}
async function launch(id,mode){
  if(pending)return;const generation=++gameGeneration;++stateGeneration;pending=true;launchingGame=true;gameReturnNotified=false;syncAudio();
  try{const result=await api('launch',{game:id,mode});if(generation!==gameGeneration)return;++stateGeneration;state.running=result.running;closeDialog();tone(true);toast(mode==='game'?'Starting your adventure…':'Opening the original game interface…');render();}
  catch(e){if(generation===gameGeneration)showError(e);}finally{if(generation===gameGeneration){pending=false;launchingGame=false;syncAudio();}}
}
function dialog(title,message,primary,onConfirm,cancel=true){
  restoreFocus=document.activeElement;dialogAction=onConfirm;
  modalRoot.innerHTML=`<div class="dialog-shade"><section class="dialog" role="dialog" aria-modal="true" aria-labelledby="dialog-title"><h2 id="dialog-title">${ball}${esc(title)}</h2><p>${esc(message)}</p><div class="dialog-actions">${cancel?'<button data-dialog="cancel">Back</button>':''}<button class="primary" data-dialog="confirm">${esc(primary)}</button></div></section></div>`;
  bindDialog();$('[data-dialog="confirm"]',modalRoot)?.focus();
}
function bindDialog(){
  $('[data-dialog="cancel"]',modalRoot)?.addEventListener('click',closeDialog);
  $('[data-dialog="confirm"]',modalRoot)?.addEventListener('click',()=>dialogAction?.());
}
function closeDialog(){modalRoot.innerHTML='';dialogAction=null;if(restoreFocus?.isConnected)restoreFocus.focus({preventScroll:true});restoreFocus=null;}
function showError(error){
  if(modalRoot.firstChild){let box=$('.error',modalRoot);if(!box){box=document.createElement('p');box.className='error';box.setAttribute('role','alert');$('.dialog-actions',modalRoot).before(box);}box.textContent=error.message;}
  else dialog('Unable to continue',error.message,'OK',closeDialog,false);
}
// Games run off the player's own ROM: choosing one extracts it (with a
// progress bar) into the game's .pkmn, then the game is ready to play.
function romDialog(){
  const g=game();
  if(g.disabled)return;
  dialog(`Import ${g.name}`,`Choose your own ${g.name} ROM (.gba). It is read once: the game's graphics, maps, music and text are extracted for the PC version. After that the ROM isn't needed.`,'Import',async()=>{
    const path=$('#import-path',modalRoot).value.trim();if(!path){showError(new Error('Choose your ROM file first.'));return;}
    const button=$('[data-dialog="confirm"]',modalRoot);button.disabled=true;
    try{await api('import',{game:g.id,path});extractProgress(g);}catch(e){showError(e);if(button.isConnected)button.disabled=false;}
  });
  $('.dialog-actions',modalRoot).insertAdjacentHTML('beforebegin',`<label for="import-path">Game ROM (.gba)</label><div class="import-path"><input id="import-path" type="text" spellcheck="false" autocomplete="off" placeholder="Choose a file on your PC"><button id="browse-file">Browse…</button></div>`);
  $('#browse-file',modalRoot).addEventListener('click',async()=>{const b=$('#browse-file',modalRoot);b.disabled=true;try{const result=await api('browse',{kind:'rom'});if(result.path&&$('#import-path',modalRoot))$('#import-path',modalRoot).value=result.path;}catch(e){showError(e);}finally{if(b.isConnected)b.disabled=false;}});
}
function extractProgress(g){
  dialogAction=null;
  modalRoot.innerHTML=`<div class="dialog-shade"><section class="dialog" role="dialog" aria-modal="true" aria-labelledby="dialog-title"><h2 id="dialog-title">${ball}Extracting ${esc(g.name)}</h2><p>Unpacking your ROM. This only happens once.</p><div class="progress" role="progressbar" aria-valuemin="0" aria-valuemax="100" aria-valuenow="0"><div></div></div><p class="progress-label">0%</p></section></div>`;
  const tick=async()=>{
    let r;
    try{r=await api('extract');}catch(e){dialog('Unable to continue',e.message,'OK',closeDialog,false);return;}
    const job=r.extract;
    if(!job||job.game!==g.id){setTimeout(tick,150);return;}
    const bar=$('.progress',modalRoot);
    if(bar){bar.firstElementChild.style.width=job.progress+'%';bar.setAttribute('aria-valuenow',job.progress);$('.progress-label',modalRoot).textContent=job.progress+'%';}
    if(!job.done){setTimeout(tick,120);return;}
    state.games=r.games;
    if(job.error){dialog(`Couldn't extract ${g.name}`,job.error,'Try again',romDialog);return;}
    render();
    dialog(`${g.name} is ready`,`Your ROM has been extracted. ${g.name} is ready to play.`,'Play',()=>launch(g.id,'game'));
  };
  tick();
}
function importDialog(kind){
  if(kind!=='save'){romDialog();return;}
  const save=kind==='save';dialog(save?'Import FireRed save':`Add ${game().name}`,save?'Choose an existing FireRed .sav file. Your current save is backed up before import.':'Choose the Windows PC executable for this game. The launcher opens the selected game from its own folder.',save?'Import save':'Add game',async()=>{
    const path=$('#import-path',modalRoot).value.trim();if(!path){showError(new Error('Choose a file first.'));return;}
    const button=$('[data-dialog="confirm"]',modalRoot);button.disabled=true;
    try{await api(save?'import-save':'import',{game:game().id,path});closeDialog();await refresh();render();toast(save?'Save imported. PC Boxes is ready.':`${game().name} added to your library.`);}catch(e){showError(e);}finally{if(button.isConnected)button.disabled=false;}
  });
  $('.dialog-actions',modalRoot).insertAdjacentHTML('beforebegin',`<label for="import-path">${save?'Save file (.sav)':'Game executable (.exe)'}</label><div class="import-path"><input id="import-path" type="text" spellcheck="false" autocomplete="off" placeholder="Choose a file on your PC"><button id="browse-file">Browse…</button></div>`);
  $('#browse-file',modalRoot).addEventListener('click',async()=>{const b=$('#browse-file',modalRoot);b.disabled=true;try{const result=await api('browse',{kind:save?'save':'game'});if(result.path&&$('#import-path',modalRoot))$('#import-path',modalRoot).value=result.path;}catch(e){showError(e);}finally{if(b.isConnected)b.disabled=false;}});
}
const optionSelect=(key,label,choices)=>`<label class="option-row"><span>${label}</span><select data-setting="${key}">${choices.map(([v,t])=>`<option value="${v}" ${Number(state.settings[key])===v?'selected':''}>${t}</option>`).join('')}</select></label>`;
const optionToggle=(key,label)=>`<label class="option-row"><span>${label}</span><input type="checkbox" data-setting="${key}" ${state.settings[key]?'checked':''}></label>`;
function settingsPanel(){
  let content='',title='';
  if(optionsTab==='game'){title='Game Settings';content=optionSelect('fast_forward','Fast-forward speed',[[2,'2×'],[3,'3×'],[4,'4×'],[5,'5×'],[6,'6×'],[7,'7×'],[8,'8×']])+'<p class="panel-note">Hold the fast-forward button (Tab or RT unless you changed it in Control Settings). Text speed, battle style, and sound mode are available in the game’s OPTIONS menu.</p>';}
  if(optionsTab==='display'){title='Display Settings';content=(!state.embedded?optionSelect('window_scale','Window size',[[1,'240 × 160'],[2,'480 × 320'],[3,'720 × 480'],[4,'960 × 640'],[5,'1200 × 800'],[6,'1440 × 960'],[7,'1680 × 1120'],[8,'1920 × 1280']]):'')+optionSelect('filter','Pixel filter',[[0,'Nearest'],[1,'Sharp'],[2,'Smooth'],[3,'LCD'],[4,'CRT']])+optionSelect('scaling','Scaling',[[0,'Fit'],[1,'Integer'],[2,'Stretch']])+optionSelect('widescreen','Field aspect ratio',[[0,'Original 3:2'],[1,'16:10'],[2,'16:9'],[3,'Match window']])+optionToggle('fullscreen','Fullscreen')+optionToggle('vsync','VSync')+optionToggle('color_correct','GBA color correction');}
  if(optionsTab==='audio'){title='Audio Settings';content=`<label class="option-row"><span>Master volume</span><span class="range-wrap"><input type="range" min="0" max="10" step="1" value="${state.settings.volume??8}" data-setting="volume"><output>${(state.settings.volume??8)*10}%</output></span></label>`+optionSelect('hq_audio','Audio quality',[[1,'High quality · 48 kHz'],[0,'Original GBA']])+'<p class="panel-note">Master volume also controls launcher music and menu sounds. Audio quality applies to your next game session.</p>';}
  if(optionsTab==='controls'){title='Control Settings';content=controlsPanel();}
  if(optionsTab==='language'){title='Language';content='<label class="option-row"><span>Game language</span><select aria-label="Game language"><option>English</option></select></label><p class="panel-note">This native build includes the original English game data.</p>';}
  const editable=['game','display','audio'].includes(optionsTab);
  return `<section class="options-panel${optionsTab==='controls'?' controls-panel':''}" aria-label="${title}"><h2>${title}</h2>${content}<div class="panel-actions"><button class="menu-button" data-action="back">Back</button>${optionsTab==='controls'?'<button class="menu-button" id="reset-binds">Default controls</button>':''}${editable?'<button class="menu-button selected" id="save-settings">Save settings</button>':''}</div></section>`;
}
// ---- controls: keyboard keys and controller buttons for each GBA button ----
// Keyboard: KeyboardEvent.code -> SDL scancode (what settings.ini stores) and its label.
const SCANCODES={Enter:[40,'Enter'],Backspace:[42,'Backspace'],Tab:[43,'Tab'],Space:[44,'Space'],Minus:[45,'-'],Equal:[46,'='],BracketLeft:[47,'['],BracketRight:[48,']'],Backslash:[49,'\\'],Semicolon:[51,';'],Quote:[52,"'"],Backquote:[53,'`'],Comma:[54,','],Period:[55,'.'],Slash:[56,'/'],CapsLock:[57,'Caps Lock'],Insert:[73,'Insert'],Home:[74,'Home'],PageUp:[75,'Page Up'],Delete:[76,'Delete'],End:[77,'End'],PageDown:[78,'Page Down'],ArrowRight:[79,'Right'],ArrowLeft:[80,'Left'],ArrowDown:[81,'Down'],ArrowUp:[82,'Up'],NumpadDivide:[84,'Keypad /'],NumpadMultiply:[85,'Keypad *'],NumpadSubtract:[86,'Keypad -'],NumpadAdd:[87,'Keypad +'],NumpadEnter:[88,'Keypad Enter'],NumpadDecimal:[99,'Keypad .'],ControlLeft:[224,'Left Ctrl'],ShiftLeft:[225,'Left Shift'],AltLeft:[226,'Left Alt'],ControlRight:[228,'Right Ctrl'],ShiftRight:[229,'Right Shift'],AltRight:[230,'Right Alt']};
for(let i=0;i<26;i++)SCANCODES['Key'+String.fromCharCode(65+i)]=[4+i,String.fromCharCode(65+i)];
for(let i=1;i<=9;i++){SCANCODES['Digit'+i]=[29+i,String(i)];SCANCODES['Numpad'+i]=[88+i,'Keypad '+i];}
SCANCODES.Digit0=[39,'0'];SCANCODES.Numpad0=[98,'Keypad 0'];
for(const f of [3,4,5,6,7,8,9,10,12])SCANCODES['F'+f]=[57+f,'F'+f];
const KEY_RESERVED=new Set(['Escape','F1','F2','F11','KeyT']);   // cancel, controls panel, filter, fullscreen, chat
const keyLabel=sc=>Object.values(SCANCODES).find(([v])=>v===sc)?.[1]??`Key ${sc}`;
// Controller: Gamepad API standard button -> SDL game controller button.
const PAD_FROM_STANDARD=[0,1,2,3,9,10,100,101,4,6,7,8,11,12,13,14,5];
const PAD_LABELS={0:'A',1:'B',2:'X',3:'Y',4:'Back',5:'Guide',6:'Start',7:'Left Stick',8:'Right Stick',9:'LB',10:'RB',11:'D-pad Up',12:'D-pad Down',13:'D-pad Left',14:'D-pad Right',100:'LT',101:'RT'};
const BIND_ROWS=[['a','A'],['b','B'],['select','Select'],['start','Start'],['up','Up'],['down','Down'],['left','Left'],['right','Right'],['l','L'],['r','R'],['turbo','Fast-forward (hold)']];
let capture=null;   // {bind:'key_a'|'pad_a', base:Set of pad buttons held when it started}
function bindCell(bind){
  const v=state.settings[bind]??state.defaults?.[bind],pad=bind.startsWith('pad_');
  const label=capture?.bind===bind?(pad?'Press a button…':'Press a key…'):pad?(PAD_LABELS[v]??`Button ${v}`):keyLabel(v);
  return `<button class="bind-cell${capture?.bind===bind?' capturing':''}" data-bind="${bind}">${label}</button>`;
}
function controlsPanel(){
  return `<table class="controls-table binds"><thead><tr><th>Button</th><th>Keyboard</th><th>Controller</th></tr></thead><tbody>${BIND_ROWS.map(([a,name])=>`<tr><td>${name}</td><td>${bindCell('key_'+a)}</td><td>${bindCell('pad_'+a)}</td></tr>`).join('')}</tbody></table><p class="panel-note">Select a box, then press the new key or button (Esc cancels). In game: F1 or hold Back + Start opens these controls; T chats online.</p>`;
}
function padsHeld(){const held=new Set();for(const p of navigator.getGamepads?.()||[])if(p)p.buttons.forEach((b,i)=>{if(b.pressed)held.add(i);});return held;}
function startCapture(bind){capture={bind,base:padsHeld()};refreshBinds(bind);}
function endCapture(){const bind=capture?.bind;capture=null;refreshBinds(bind);}
function refreshBinds(focus){
  const table=$('.binds',screen);if(!table)return;
  table.outerHTML=controlsPanel().replace(/<p class="panel-note">[\s\S]*$/,'');
  bindControls();$(`[data-bind="${focus}"]`,screen)?.focus();
}
async function saveBind(bind,value){
  const settings={...state.defaults,...state.settings},old=settings[bind],kind=bind.slice(0,4);
  // whatever already had this key/button gets this one's old one (a swap)
  for(const[a]of BIND_ROWS)if(kind+a!==bind&&settings[kind+a]===value)settings[kind+a]=old;
  settings[bind]=value;capture=null;
  try{const result=await api('settings',{settings});state.settings=result.settings;tone(true);}catch(e){showError(e);}
  refreshBinds(bind);
}
function captureKey(event){
  event.preventDefault();if(event.repeat)return;
  if(event.code==='Escape'){endCapture();return;}
  if(!capture.bind.startsWith('key_'))return;
  const sc=SCANCODES[event.code];
  if(!sc||KEY_RESERVED.has(event.code)){toast('That key is used for something else. Pick another.');return;}
  saveBind(capture.bind,sc[0]);
}
function capturePad(){
  const held=padsHeld();
  for(const i of [...capture.base])if(!held.has(i))capture.base.delete(i);   // let go: can count again
  const fresh=[...held].find(i=>!capture.base.has(i));
  if(fresh===undefined)return;
  if(capture.bind.startsWith('pad_')&&PAD_FROM_STANDARD[fresh]!==undefined)saveBind(capture.bind,PAD_FROM_STANDARD[fresh]);
  else if(fresh===1)endCapture();   // B cancels a keyboard capture
  else capture.base.add(fresh);
}
function moveBindFocus(key){
  const items=[...$$('.bind-cell',screen),$('#reset-binds',screen),$('[data-action="back"]',screen)].filter(Boolean);
  let i=items.indexOf(document.activeElement),cells=$$('.bind-cell',screen).length;
  if(i<0){items[0]?.focus();return;}
  if(key==='ArrowUp')i=i>=cells?cells-1:i-2;else if(key==='ArrowDown')i=i<cells?i+2:i;else if(key==='ArrowLeft')i--;else if(key==='ArrowRight')i++;
  items[Math.max(0,Math.min(items.length-1,i))]?.focus();
}
function bindControls(){$$('.bind-cell',screen).forEach(b=>b.addEventListener('click',()=>startCapture(b.dataset.bind)));}
async function resetBinds(){
  const settings={...state.defaults,...state.settings};
  for(const[a]of BIND_ROWS){settings['key_'+a]=state.defaults['key_'+a];settings['pad_'+a]=state.defaults['pad_'+a];}
  try{const result=await api('settings',{settings});state.settings=result.settings;tone(true);toast('Default controls restored.');refreshBinds();}catch(e){showError(e);}
}
function bindSettings(){ bindControls();$('#reset-binds')?.addEventListener('click',resetBinds);$('#save-settings')?.addEventListener('click',saveSettings);$$('input[type=range]',screen).forEach(i=>i.addEventListener('input',()=>{i.nextElementSibling.textContent=`${i.value*10}%`;if(i.dataset.setting==='volume'){previewVolume=Number(i.value);syncAudio();}}));}
async function saveSettings(){const settings={...state.settings};$$('[data-setting]',screen).forEach(input=>settings[input.dataset.setting]=input.type==='checkbox'?Number(input.checked):Number(input.value));try{const result=await api('settings',{settings});state.settings=result.settings;previewVolume=null;syncAudio();tone(true);toast('Settings saved. They will apply when you next open the game.');}catch(e){showError(e);}}
function navigate(key){
  if(navigationPending||sceneTransitions?.busy)return;
  if(modalRoot.firstChild){if(key==='Escape'||key==='back'){tone();closeDialog();return;}if(key==='confirm'){tone();dialogAction?.();return;}return;}
  if(route==='notice'){if(key==='confirm')go('title');return;}
  if(route==='title'){go('main');return;}
  if(route==='storage'&&storageUI){if(storageUI.key(key==='confirm'?'Enter':key==='back'?'Escape':key)){tone();return;}}
  if(optionsTab==='controls'&&key.startsWith?.('Arrow')){moveBindFocus(key);return;}
  if(key==='Escape'||key==='back'){back();return;}if(key==='confirm'){confirm();return;}
  if(route==='games'&&['ArrowLeft','ArrowRight'].includes(key)){selectGame(gameIndex+(key==='ArrowRight'?1:-1));return;}
  if(['ArrowUp','ArrowDown'].includes(key)&&!optionsTab){selectMenu(selected+(key==='ArrowDown'?1:-1));return;}
}
document.addEventListener('keydown',event=>{
  if(capture){captureKey(event);return;}
  if(['INPUT','SELECT','TEXTAREA'].includes(event.target.tagName)){if(event.key==='Escape'){event.preventDefault();back();}return;}
  if(modalRoot.firstChild&&event.key==='Tab'){const focusable=$$('button,input,select',modalRoot).filter(x=>!x.disabled);const first=focusable[0],last=focusable.at(-1);if(event.shiftKey&&document.activeElement===first){event.preventDefault();last.focus();}else if(!event.shiftKey&&document.activeElement===last){event.preventDefault();first.focus();}return;}
  if(event.repeat)return;
  if(route==='title'&&!['Tab','Shift','Control','Alt','Meta','F11','F12'].includes(event.key)){event.preventDefault();navigate('confirm');return;}
  if(['ArrowUp','ArrowDown','ArrowLeft','ArrowRight','PageUp','PageDown','Escape','Backspace','Enter','x','X','z','Z'].includes(event.key)){
    if(event.key==='Enter'&&document.activeElement?.tagName==='BUTTON')return;
    if(optionsTab&&optionsTab!=='controls'&&['ArrowLeft','ArrowRight','ArrowUp','ArrowDown'].includes(event.key))return;
    event.preventDefault();navigate(['x','X','Enter'].includes(event.key)?'confirm':['z','Z','Backspace'].includes(event.key)?'back':event.key);
  }
});
document.addEventListener('click',event=>{if(event.target.closest?.('button')&&event.target.closest('#stage'))tone();},true);
let lastPadTime=0,lastPadAction='';
function gamepads(time){
  if(document.hidden||state.running){lastPadAction='';requestAnimationFrame(gamepads);return;}
  if(capture){capturePad();lastPadAction='busy';requestAnimationFrame(gamepads);return;}
  const pad=[...(navigator.getGamepads?.()||[])].find(Boolean);
  if(pad){const down=i=>pad.buttons[i]?.pressed;const key=down(0)?'confirm':down(1)?'back':down(4)?'PageUp':down(5)?'PageDown':down(12)||pad.axes[1]<-.55?'ArrowUp':down(13)||pad.axes[1]>.55?'ArrowDown':down(14)||pad.axes[0]<-.55?'ArrowLeft':down(15)||pad.axes[0]>.55?'ArrowRight':'';
    if(key&&(key!==lastPadAction||(time-lastPadTime>230&&key.startsWith('Arrow')))){launcherAudio?.unlock();navigate(key);lastPadTime=time;}lastPadAction=key;
  }requestAnimationFrame(gamepads);
}
const transitionsReady=import('./assets/screen-transitions.js').then(({createScreenTransitions})=>{sceneTransitions=createScreenTransitions($('#screen-transition'));return sceneTransitions.reveal();}).catch(error=>{console.error('Screen transitions unavailable',error);$('#screen-transition').style.opacity=0;$('#screen-transition').style.pointerEvents='none';});
const audioReady=import('./assets/audio.js').then(({createLauncherAudio})=>{launcherAudio=createLauncherAudio();syncAudio();}).catch(error=>console.warn('Launcher audio unavailable',error));
render();refresh(true);statusTimer=setInterval(()=>refresh(),2200);requestAnimationFrame(gamepads);
for(const name of ['main','games','boxes','gifts','options']){const img=new Image();img.src=`assets/backgrounds/${name}.png`;}
for(const path of ['logo.png','characters/charizard-official.png','characters/venusaur-official.png']){const img=new Image();img.src=`assets/${path}`;}
