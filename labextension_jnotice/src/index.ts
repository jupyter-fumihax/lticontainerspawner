// src/index.ts  (jnotice.js)

import { JupyterFrontEnd, JupyterFrontEndPlugin } from '@jupyterlab/application';

const POLL_MS = 60_000; // ポーリング間隔（ミリ秒

const plugin: JupyterFrontEndPlugin<void> = {
  id: 'jnotice:plugin',
  autoStart: true,
  activate: async (_app: JupyterFrontEnd) => {
    // JupyterHub/Lab の baseUrl 推定
    const cfgEl = document.getElementById('jupyter-config-data');
    const cfg = cfgEl && cfgEl.textContent ? JSON.parse(cfgEl.textContent) : {};
    const base: string = (cfg.baseUrl || (location.pathname.match(/\/user\/[^/]+\/?/)?.[0] || '/')).replace(/\/?$/, '/');

    // XSRF 対応
    const xsrf = (document.cookie.match(/(?:^|; )_xsrf=([^;]+)/)?.[1] || '');
    const url = base + 'files/.jnotice.txt';

    let last = '';

    async function poll() {
      try {
        const r = await fetch(url + '?_=' + Date.now(), {
          credentials: 'same-origin',
          headers: xsrf ? { 'X-XSRFToken': xsrf } : {}
        });
        if (!r.ok) return;
        const t = (await r.text()).trim();
        if (t && t !== last) {
          last = t;
          toast(t);
        }
      } catch {
        // スキップ
      }
    }


    function toast(msg: string) {
      const el = document.createElement('div');
      el.setAttribute('role', 'status');
      el.setAttribute('aria-live', 'polite');
      el.style.cssText = [
        'position:fixed',
        'right:20px',
        'bottom:20px',
        'background:#ffefc6',
        'color:#222',
        'padding:10px 14px',
        'border-radius:10px',
        'box-shadow:0 2px 8px rgba(0,0,0,.2)',
        'z-index:9999',
        'max-width:360px',
        'display:flex',
        'align-items:center',
        'gap:8px'
      ].join(';') + ';';

      const text = document.createElement('span');
      text.textContent = msg;
      text.style.cssText = 'line-height:1.4;';
      el.appendChild(text);

      // 閉じるボタン（無期限表示）
      const btn = document.createElement('button');
      btn.type = 'button';
      btn.setAttribute('aria-label', '閉じる');
      btn.textContent = '×';
      btn.style.cssText = [
        'border:none',
        'background:transparent',
        'font-size:16px',
        'cursor:pointer',
        'line-height:1',
        'padding:4px',
        'margin-left:auto',
        'border-radius:6px'
      ].join(';') + ';';
      btn.onmouseenter = () => (btn.style.backgroundColor = 'rgba(0,0,0,.06)');
      btn.onmouseleave = () => (btn.style.backgroundColor = 'transparent');
      btn.onclick = () => el.remove();

      el.appendChild(btn);
      document.body.appendChild(el);
    }

    // POLL_MS 間隔でポーリング
    setTimeout(() => { poll(); setInterval(poll, POLL_MS); }, 1000);

    console.log(`[jnotice] plugin loaded; polling every ${POLL_MS} ms`);
  }
};

export default plugin;
