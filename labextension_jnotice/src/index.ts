// src/index.ts  (jnotice.js)

import { JupyterFrontEnd, JupyterFrontEndPlugin } from '@jupyterlab/application';

const DEF_POLL_MS = 60_000;           // ポーリング間隔（ミリ秒）
const NOTICE_PATH = '.jnotice.txt';   // メッセージファイル

const plugin: JupyterFrontEndPlugin<void> = {
  id: 'jnotice:plugin',
  autoStart: true,
  activate: async (_app: JupyterFrontEnd) => {
    //
    const cfgEl = document.getElementById('jupyter-config-data');
    const raw   = cfgEl?.textContent ?? '';
    const cfg   = raw ? JSON.parse(raw) : {};

    const jc = (
        (cfg as any).jnotice ??
        (cfg as any).page_config_data?.jnotice ??
        (cfg as any).pageConfig?.jnotice ??
        {}
    ) as { path?: string; pollMs?: number }

    const base: string = (cfg.baseUrl || (location.pathname.match(/\/user\/[^/]+\/?/)?.[0] || '/')).replace(/\/?$/, '/');
    const noticePath   = (typeof jc.path === 'string' && jc.path.trim()) ? jc.path : NOTICE_PATH;
    const pollMs       = Number.isFinite(jc.pollMs) ? Number(jc.pollMs) : DEF_POLL_MS;

    // XSRF 対応
    const xsrf = (document.cookie.match(/(?:^|; )_xsrf=([^;]+)/)?.[1] || '');
    const url = base + 'files/' + noticePath.replace(/^\/+/, '');

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
        // Skip
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

      // Close Button
      const btn = document.createElement('button');
      btn.type = 'button';
      btn.setAttribute('aria-label', 'close');
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

    // ポーリング
    setTimeout(() => { poll(); setInterval(poll, pollMs); }, 1000);

    console.log(`[jnotice] plugin loaded; polling "${noticePath}" every ${pollMs} ms`);
  }
};

export default plugin;
