/** Persistent launcher music and UI sound; independent of screen DOM replacement. */
export function createLauncherAudio() {
  const menuScreens = new Set(['main', 'games', 'boxes', 'storage', 'gifts', 'options']);
  const musicGain = .25, selectGain = .55, fadeInMs = 1200;
  const root = document.createElement('div');
  root.hidden = true;
  root.dataset.launcherAudio = '';
  document.body.appendChild(root);
  const media = {};
  const reported = new Set();
  const pending = new Map();
  let screen = 'notice', running = false, volume = .8, current = null;
  let envelope = 0, fading = 0, transitioning = false, disposed = false, suspended = false;
  let lastSelect = -Infinity;

  const report = (name, error) => {
    if (disposed || reported.has(name)) return;
    reported.add(name);
    console.warn(`Launcher audio unavailable: ${name}`, error);
  };
  for (const [name, file] of Object.entries({title:'title_screen.mp3', menu:'menus_music.mp3', select:'select.mp3'})) {
    const audio = document.createElement('audio');
    audio.dataset.audioTrack = name;
    audio.preload = 'auto';
    audio.loop = name !== 'select';
    audio.volume = 0;
    audio.src = new URL(`./sounds/${file}`, import.meta.url).href;
    audio.addEventListener('error', () => report(file, audio.error?.message || 'Could not decode this sound.'));
    root.appendChild(audio);
    media[name] = audio;
    audio.load();
  }

  const audible = () => !disposed && !suspended && !document.hidden && !running && volume > 0;
  const musicFor = route => route === 'title' ? 'title' : menuScreens.has(route) ? 'menu' : null;
  const wanted = () => musicFor(screen);
  function applyVolume() {
    for (const name of ['title', 'menu']) media[name].volume = name === current ? Math.max(0, Math.min(1, volume * envelope * musicGain)) : 0;
    media.select.volume = volume * selectGain;
  }
  function fade(target, duration = fadeInMs) {
    cancelAnimationFrame(fading);
    fading = 0;
    if (disposed) return;
    const from = envelope, start = performance.now();
    if (!duration || document.hidden || from === target) {
      envelope = target;
      applyVolume();
      return;
    }
    const step = now => {
      const progress = Math.max(0, Math.min(1, (now - start) / duration));
      envelope = from + (target - from) * progress;
      applyVolume();
      fading = progress < 1 ? requestAnimationFrame(step) : 0;
    };
    fading = requestAnimationFrame(step);
  }
  function play(name) {
    if (pending.has(name)) return pending.get(name);
    const audio = media[name];
    let promise;
    try { promise = Promise.resolve(audio.play()); }
    catch (error) { promise = Promise.reject(error); }
    const request = promise.then(() => {
      if (!audible() || (name !== 'select' && (name !== current || name !== wanted()))) audio.pause();
      return !audio.paused;
    }).catch(error => {
      // Autoplay can be denied until a real gesture. The next gesture retries.
      if (!['NotAllowedError', 'AbortError'].includes(error?.name)) report(name, error);
      return false;
    }).finally(() => {
      if (pending.get(name) === request) pending.delete(name);
    });
    pending.set(name, request);
    return request;
  }
  function sync() {
    const next = wanted();
    if (next !== current) {
      media.title.pause();
      media.menu.pause();
      current = next;
      fade(0, 0);
    }
    if (!audible() || !current) {
      media.title.pause();
      media.menu.pause();
      media.select.pause();
      fade(0, 0);
      return;
    }
    applyVolume();
    if (media[current].paused) {
      const name = current;
      play(name).then(playing => {
        if (playing && current === name && !transitioning && audible()) fade(1);
      });
    }
  }
  function unlock() {
    if (disposed) return;
    // A gesture on the quiet notice still authorizes later media playback in
    // Chromium. Starting the selected track here keeps the call in that gesture.
    sync();
  }
  function visibilityChanged() { sync(); }
  function pageHidden() { suspended = true; sync(); }
  function pageShown() { suspended = false; sync(); }
  document.addEventListener('pointerdown', unlock, true);
  document.addEventListener('keydown', unlock, true);
  document.addEventListener('visibilitychange', visibilityChanged);
  window.addEventListener('pagehide', pageHidden);
  window.addEventListener('pageshow', pageShown);

  return {
    setState({route = screen, playing = running, masterVolume = volume * 10} = {}) {
      screen = route;
      running = Boolean(playing);
      const number = Number(masterVolume);
      volume = Number.isFinite(number) ? Math.max(0, Math.min(10, number)) / 10 : .8;
      sync();
    },
    setVolume(value) { this.setState({masterVolume:value}); },
    unlock,
    select() {
      if (!audible() || screen === 'notice') return;
      const now = performance.now();
      if (now - lastSelect < 70) return;
      lastSelect = now;
      const sound = media.select;
      sound.pause();
      pending.delete('select');
      try { sound.currentTime = 0; } catch {}
      sound.volume = volume * selectGain;
      play('select');
    },
    beginTransition(nextScreen = screen) {
      if (musicFor(nextScreen) === wanted()) return;
      transitioning = true;
      fade(0, 160);
      if (current && !media[current].paused) return new Promise(resolve => setTimeout(resolve, 160));
    },
    endTransition() {
      const wasTransitioning = transitioning;
      transitioning = false;
      sync();
      if (wasTransitioning && audible() && current && !media[current].paused && !pending.has(current)) fade(1);
    },
    dispose() {
      if (disposed) return;
      disposed = true;
      cancelAnimationFrame(fading);
      document.removeEventListener('pointerdown', unlock, true);
      document.removeEventListener('keydown', unlock, true);
      document.removeEventListener('visibilitychange', visibilityChanged);
      window.removeEventListener('pagehide', pageHidden);
      window.removeEventListener('pageshow', pageShown);
      for (const audio of Object.values(media)) {
        audio.pause();
        audio.removeAttribute('src');
        audio.load();
      }
      root.remove();
    }
  };
}
