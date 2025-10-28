const path = require('path');
const webpack = require('webpack');
const { container } = webpack;
const { ModuleFederationPlugin } = container;

module.exports = {
  output: {
    path: path.resolve(__dirname, 'static'),
    publicPath: 'auto'
  },
  resolve: { extensions: ['.ts', '.js'] },
  module: { rules: [{ test: /\.ts$/, use: 'ts-loader', exclude: /node_modules/ }] },
  plugins: [
    new webpack.BannerPlugin({ banner: 'window._JUPYTERLAB = window._JUPYTERLAB || {};', raw: true }),
    new ModuleFederationPlugin({
      name: 'jnotice',
      filename: 'remoteEntry.jnotice.js',
      shareScope: 'jupyterlab',
      library: { type: 'assign', name: 'window._JUPYTERLAB["jnotice"]' },
      exposes: { './extension': './src/index.ts' },
      shared: {
        '@jupyterlab/application': {
          singleton: true, requiredVersion: '^4.0.0',
          import: false, shareKey: '@jupyterlab/application', shareScope: 'jupyterlab'
        },
        '@jupyterlab/ui-components': {
          singleton: true, requiredVersion: '^4.0.0',
          import: false, shareKey: '@jupyterlab/ui-components', shareScope: 'jupyterlab'
        },
        '@lumino/widgets': {
          singleton: true, requiredVersion: '^2.0.0',
          import: false, shareKey: '@lumino/widgets', shareScope: 'jupyterlab'
        }
      }
    })
  ],
  mode: 'production',
  optimization: { minimize: true }
};
