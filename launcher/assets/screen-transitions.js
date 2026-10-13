/** Keep one stable black overlay outside the screen content being replaced. */
export function createScreenTransitions(overlay) {
  const doc = overlay?.ownerDocument;
  const win = doc?.defaultView;
  if (!overlay?.style || !win) throw new TypeError('Screen transitions need an overlay element.');

  const reducedMotion = win.matchMedia?.('(prefers-reduced-motion: reduce)');
  let disposed = false;
  let busy = false;
  let generation = 0;
  let animation = null;
  const pendingFrames = new Set();
  const current = token => !disposed && token === generation;

  function reset() {
    animation?.cancel();
    animation = null;
    overlay.style.opacity = '0';
    overlay.style.pointerEvents = 'none';
    busy = false;
  }

  async function fade(opacity, duration, token) {
    if (!current(token)) return false;
    if (reducedMotion?.matches || doc.hidden || !overlay.animate) {
      overlay.style.opacity = String(opacity);
      return true;
    }
    const from = Number.parseFloat(win.getComputedStyle(overlay).opacity);
    const active = overlay.animate(
      [{opacity: Number.isFinite(from) ? from : 0}, {opacity}],
      {duration, easing: 'ease-in-out', fill: 'forwards'}
    );
    animation = active;
    try {
      await active.finished;
      if (!current(token)) return false;
      overlay.style.opacity = String(opacity);
      return true;
    } catch (error) {
      if (error?.name === 'AbortError' || !current(token)) return false;
      throw error;
    } finally {
      active.cancel();
      if (animation === active) animation = null;
    }
  }

  function renderFrame(token) {
    if (!current(token) || doc.hidden) return Promise.resolve(current(token));
    return new Promise(resolve => {
      let frame;
      let timeout;
      const finish = () => {
        win.cancelAnimationFrame(frame);
        win.clearTimeout(timeout);
        pendingFrames.delete(finish);
        resolve(current(token));
      };
      pendingFrames.add(finish);
      frame = win.requestAnimationFrame(finish);
      // A backgrounded window can stop delivering animation frames mid-swap.
      timeout = win.setTimeout(finish, 100);
    });
  }

  async function transition(swap, initialReveal) {
    if (disposed || busy) return false;
    const token = ++generation;
    busy = true;
    overlay.style.pointerEvents = 'auto';
    try {
      if (initialReveal) {
        overlay.style.opacity = '1';
        return await fade(0, 350, token);
      }
      if (!await fade(1, 160, token)) return false;
      await swap();
      if (!current(token)) return false;
      if (!await renderFrame(token) || !await renderFrame(token)) return false;
      return await fade(0, 240, token);
    } finally {
      if (token === generation) reset();
    }
  }

  // Honor a preference change immediately, including during an active fade.
  const onMotionChange = () => {
    if (reducedMotion?.matches && animation) animation.finish();
  };
  reducedMotion?.addEventListener?.('change', onMotionChange);

  return {
    get busy() { return busy; },
    run(swap) {
      if (typeof swap !== 'function') throw new TypeError('Screen transitions need a swap callback.');
      return transition(swap, false);
    },
    reveal() { return transition(null, true); },
    reset() {
      if (disposed) return;
      ++generation;
      for (const finish of pendingFrames) finish();
      reset();
    },
    dispose() {
      if (disposed) return;
      disposed = true;
      ++generation;
      for (const finish of pendingFrames) finish();
      reducedMotion?.removeEventListener?.('change', onMotionChange);
      reset();
    }
  };
}
