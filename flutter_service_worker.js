'use strict';
const MANIFEST = 'flutter-app-manifest';
const TEMP = 'flutter-temp-cache';
const CACHE_NAME = 'flutter-app-cache';

const RESOURCES = {"404.html": "b211b34025625fceb1ca76f91f040331",
"assets/AssetManifest.bin": "12585b06667667b29820f74a159bfab2",
"assets/AssetManifest.bin.json": "fb118e5b2ac419c627e2f44e8db8d456",
"assets/AssetManifest.json": "8d08fc1f111037f19a3498a7371a9c76",
"assets/assets/environment": "450c7bcfb8df8824765d8e371dd100d3",
"assets/assets/font/GoogleSans-Regular.ttf": "bcacfd36981bfff231992d23ad1df614",
"assets/assets/images/dark/launcher.png": "4e7992175fe3b428c88570ddd3679988",
"assets/assets/images/dark/phone.jpg": "6b7abfda0fb4fbd32f0ba29f8adf4a1f",
"assets/assets/images/icons/addendum.png": "953383c0e9643fd4768048ad2bdc45f1",
"assets/assets/images/icons/altverz.png": "7fb02d22fc43128038732a52b0bed886",
"assets/assets/images/icons/fallen.png": "61e3c1f9a02b082d05d7051607210485",
"assets/assets/images/icons/reimagined.png": "2c9df39d8a57350827509a2f3d2d1e12",
"assets/assets/images/icons/requiem.png": "6e25caeeab0f3535daf9d2b21cad0ccd",
"assets/assets/images/icons/resonance.png": "71492d29bbb787a4a0854039c9432185",
"assets/assets/images/light/launcher.png": "fb419ef747fd1b0b9032ba7f23b0db15",
"assets/assets/images/light/phone.jpg": "076cc569d443592d5acec5f6468bef88",
"assets/assets/images/logo.png": "3fbca5ed7a8d288e61b10d0a5b590860",
"assets/assets/images/logos/android.png": "7f2ca7fe121c2cd77ac95d91254ebdd6",
"assets/assets/images/logos/windows.png": "c087aaaa32c2773641819d26727494ae",
"assets/assets/images/phone_view.jpg": "1586c72be75677ea910da5813ba35505",
"assets/assets/images/terminal.png": "6dd8f21a6724aa6caa14eb59422f1748",
"assets/FontManifest.json": "bfca3822ac08ac3d5c59e7370c134489",
"assets/fonts/MaterialIcons-Regular.otf": "4259706719145123e8f8e3cc4cef3a40",
"assets/NOTICES": "7b496bdec978f2b8fee90d43bc25837a",
"assets/packages/cupertino_icons/assets/CupertinoIcons.ttf": "33b7d9392238c04c131b6ce224e13711",
"assets/packages/material_symbols_icons/lib/fonts/MaterialSymbolsOutlined.ttf": "e264ca521624d43d7d261f8eb4817ccc",
"assets/packages/material_symbols_icons/lib/fonts/MaterialSymbolsRounded.ttf": "4f2b2eb12ef2118346efc421616d5201",
"assets/packages/material_symbols_icons/lib/fonts/MaterialSymbolsSharp.ttf": "1075e0f9b35c08103f17144b1e97c073",
"assets/shaders/ink_sparkle.frag": "ecc85a2e95f5e9f53123dcaf8cb9b6ce",
"canvaskit/canvaskit.js": "86e461cf471c1640fd2b461ece4589df",
"canvaskit/canvaskit.js.symbols": "68eb703b9a609baef8ee0e413b442f33",
"canvaskit/canvaskit.wasm": "efeeba7dcc952dae57870d4df3111fad",
"canvaskit/chromium/canvaskit.js": "34beda9f39eb7d992d46125ca868dc61",
"canvaskit/chromium/canvaskit.js.symbols": "5a23598a2a8efd18ec3b60de5d28af8f",
"canvaskit/chromium/canvaskit.wasm": "64a386c87532ae52ae041d18a32a3635",
"canvaskit/skwasm.js": "f2ad9363618c5f62e813740099a80e63",
"canvaskit/skwasm.js.symbols": "80806576fa1056b43dd6d0b445b4b6f7",
"canvaskit/skwasm.wasm": "f0dfd99007f989368db17c9abeed5a49",
"canvaskit/skwasm_st.js": "d1326ceef381ad382ab492ba5d96f04d",
"canvaskit/skwasm_st.js.symbols": "c7e7aac7cd8b612defd62b43e3050bdd",
"canvaskit/skwasm_st.wasm": "56c3973560dfcbf28ce47cebe40f3206",
"favicon.png": "e99f09d70e30c54528c3aa81d80aaa88",
"flutter.js": "76f08d47ff9f5715220992f993002504",
"flutter_bootstrap.js": "6f238dd46e4868a57d3eb2dd40ff9923",
"icons/Icon-192.png": "7be4fc11a7bf1929e616b54714c1687d",
"icons/Icon-512.png": "ee10b285308acce72ab318a138cf4edb",
"icons/Icon-maskable-192.png": "7be4fc11a7bf1929e616b54714c1687d",
"icons/Icon-maskable-512.png": "ee10b285308acce72ab318a138cf4edb",
"index.html": "251b62eb6c77785641536b52ada59f21",
"/": "251b62eb6c77785641536b52ada59f21",
"main.dart.js": "9abae60863d2d0c8fdc05796ec25be5a",
"manifest.json": "dc9237b23dfbaf63a23153bc4e4c27c4",
"script.js": "4fdb73f39f09e2f9931361181fc4fca3",
"styles.css": "823a83dd1efc5febe894cd370fb2509d",
"version.json": "980547175e325fe622a3362b84d55b6a"};
// The application shell files that are downloaded before a service worker can
// start.
const CORE = ["main.dart.js",
"index.html",
"flutter_bootstrap.js",
"assets/AssetManifest.bin.json",
"assets/FontManifest.json"];

// During install, the TEMP cache is populated with the application shell files.
self.addEventListener("install", (event) => {
  self.skipWaiting();
  return event.waitUntil(
    caches.open(TEMP).then((cache) => {
      return cache.addAll(
        CORE.map((value) => new Request(value, {'cache': 'reload'})));
    })
  );
});
// During activate, the cache is populated with the temp files downloaded in
// install. If this service worker is upgrading from one with a saved
// MANIFEST, then use this to retain unchanged resource files.
self.addEventListener("activate", function(event) {
  return event.waitUntil(async function() {
    try {
      var contentCache = await caches.open(CACHE_NAME);
      var tempCache = await caches.open(TEMP);
      var manifestCache = await caches.open(MANIFEST);
      var manifest = await manifestCache.match('manifest');
      // When there is no prior manifest, clear the entire cache.
      if (!manifest) {
        await caches.delete(CACHE_NAME);
        contentCache = await caches.open(CACHE_NAME);
        for (var request of await tempCache.keys()) {
          var response = await tempCache.match(request);
          await contentCache.put(request, response);
        }
        await caches.delete(TEMP);
        // Save the manifest to make future upgrades efficient.
        await manifestCache.put('manifest', new Response(JSON.stringify(RESOURCES)));
        // Claim client to enable caching on first launch
        self.clients.claim();
        return;
      }
      var oldManifest = await manifest.json();
      var origin = self.location.origin;
      for (var request of await contentCache.keys()) {
        var key = request.url.substring(origin.length + 1);
        if (key == "") {
          key = "/";
        }
        // If a resource from the old manifest is not in the new cache, or if
        // the MD5 sum has changed, delete it. Otherwise the resource is left
        // in the cache and can be reused by the new service worker.
        if (!RESOURCES[key] || RESOURCES[key] != oldManifest[key]) {
          await contentCache.delete(request);
        }
      }
      // Populate the cache with the app shell TEMP files, potentially overwriting
      // cache files preserved above.
      for (var request of await tempCache.keys()) {
        var response = await tempCache.match(request);
        await contentCache.put(request, response);
      }
      await caches.delete(TEMP);
      // Save the manifest to make future upgrades efficient.
      await manifestCache.put('manifest', new Response(JSON.stringify(RESOURCES)));
      // Claim client to enable caching on first launch
      self.clients.claim();
      return;
    } catch (err) {
      // On an unhandled exception the state of the cache cannot be guaranteed.
      console.error('Failed to upgrade service worker: ' + err);
      await caches.delete(CACHE_NAME);
      await caches.delete(TEMP);
      await caches.delete(MANIFEST);
    }
  }());
});
// The fetch handler redirects requests for RESOURCE files to the service
// worker cache.
self.addEventListener("fetch", (event) => {
  if (event.request.method !== 'GET') {
    return;
  }
  var origin = self.location.origin;
  var key = event.request.url.substring(origin.length + 1);
  // Redirect URLs to the index.html
  if (key.indexOf('?v=') != -1) {
    key = key.split('?v=')[0];
  }
  if (event.request.url == origin || event.request.url.startsWith(origin + '/#') || key == '') {
    key = '/';
  }
  // If the URL is not the RESOURCE list then return to signal that the
  // browser should take over.
  if (!RESOURCES[key]) {
    return;
  }
  // If the URL is the index.html, perform an online-first request.
  if (key == '/') {
    return onlineFirst(event);
  }
  event.respondWith(caches.open(CACHE_NAME)
    .then((cache) =>  {
      return cache.match(event.request).then((response) => {
        // Either respond with the cached resource, or perform a fetch and
        // lazily populate the cache only if the resource was successfully fetched.
        return response || fetch(event.request).then((response) => {
          if (response && Boolean(response.ok)) {
            cache.put(event.request, response.clone());
          }
          return response;
        });
      })
    })
  );
});
self.addEventListener('message', (event) => {
  // SkipWaiting can be used to immediately activate a waiting service worker.
  // This will also require a page refresh triggered by the main worker.
  if (event.data === 'skipWaiting') {
    self.skipWaiting();
    return;
  }
  if (event.data === 'downloadOffline') {
    downloadOffline();
    return;
  }
});
// Download offline will check the RESOURCES for all files not in the cache
// and populate them.
async function downloadOffline() {
  var resources = [];
  var contentCache = await caches.open(CACHE_NAME);
  var currentContent = {};
  for (var request of await contentCache.keys()) {
    var key = request.url.substring(origin.length + 1);
    if (key == "") {
      key = "/";
    }
    currentContent[key] = true;
  }
  for (var resourceKey of Object.keys(RESOURCES)) {
    if (!currentContent[resourceKey]) {
      resources.push(resourceKey);
    }
  }
  return contentCache.addAll(resources);
}
// Attempt to download the resource online before falling back to
// the offline cache.
function onlineFirst(event) {
  return event.respondWith(
    fetch(event.request).then((response) => {
      return caches.open(CACHE_NAME).then((cache) => {
        cache.put(event.request, response.clone());
        return response;
      });
    }).catch((error) => {
      return caches.open(CACHE_NAME).then((cache) => {
        return cache.match(event.request).then((response) => {
          if (response != null) {
            return response;
          }
          throw error;
        });
      });
    })
  );
}
