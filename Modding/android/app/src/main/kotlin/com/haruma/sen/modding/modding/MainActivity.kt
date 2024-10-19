package com.haruma.sen.environment

import androidx.annotation.NonNull
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodCall
import android.Manifest
import android.content.ContentResolver
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.Environment
import android.provider.DocumentsContract
import android.provider.OpenableColumns
import android.provider.Settings
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import java.io.File

class MainActivity: FlutterActivity() {

    private val channel = "com.haruma.sen.environment"

    private val REQUEST_PICK_STORAGE_ITEM: Int = 1001

    private val REQUEST_REQUEST_EXTERNAL_STORAGE_PERMISSION: Int = 1002

    private val continuation = Channel<Any?>()

    override fun configureFlutterEngine(@NonNull flutterEngine: FlutterEngine) {
        super.configureFlutterEngine(flutterEngine)
        MethodChannel(
            flutterEngine.dartExecutor.binaryMessenger,
            channel
        ).setMethodCallHandler { call, result ->
            CoroutineScope(Dispatchers.Main).launch { this@MainActivity.handle(call, result) }
			return@setMethodCallHandler
        }
    }

    public override fun onActivityResult(
		requestCode: Int,
		resultCode: Int,
		data: Intent?,
	): Unit {
		super.onActivityResult(requestCode, resultCode, data)
		this.registerOnActivityResult(requestCode, resultCode, data)
		return
	}

	public override fun onRequestPermissionsResult(
		requestCode: Int,
		permissions: Array<out String>,
		grantResults: IntArray,
	): Unit {
		super.onRequestPermissionsResult(requestCode, permissions, grantResults)
		this.registerOnRequestPermissionsResult(requestCode, permissions, grantResults)
		return
	}

    public fun registerOnActivityResult(
		requestCode: Int,
		resultCode: Int,
		data: Intent?,
	): Unit {
		when (requestCode) {
			REQUEST_PICK_STORAGE_ITEM -> {
				runBlocking {
					this@MainActivity.continuation.send(data?.data)
				}
			}
			REQUEST_REQUEST_EXTERNAL_STORAGE_PERMISSION -> {
				runBlocking {
					this@MainActivity.continuation.send(null)
				}
			}
		}
		return
	}

	public fun registerOnRequestPermissionsResult(
		requestCode: Int,
		permissions: Array<out String>,
		grantResults: IntArray,
	) {
		when (requestCode) {
			REQUEST_REQUEST_EXTERNAL_STORAGE_PERMISSION -> {
				runBlocking {
					this@MainActivity.continuation.send(null)
				}
			}
		}
		return
	}
    
    private suspend fun handle(
        call: MethodCall,
        result: MethodChannel.Result,
    ) {
        try {
            when (call.method) {
                "pick_file" -> {
                    val destination = Uri.parse(this.pickStorageFileFromDocument())
                    result.success(resolveUri(destination))
                }
                "pick_directory" -> {
                    val destination = Uri.parse(this.pickDirectoryFromDocument())
                    result.success(resolveUri(destination))
                }
                "request_storage_permission" -> {
                    val hasPermission = this@MainActivity.requestStoragePermission()
                    result.success(hasPermission)
                }
                "check_storage_permission" -> {
                    val hasPermission = this@MainActivity.checkStoragePermission()
                    result.success(hasPermission)
                }
                else -> {
                    result.notImplemented()
                }
            }
        } catch (e: Exception) {
            result.error("", e.stackTraceToString(), null)
        }
    }

    private suspend fun pickStorageFileFromDocument(): String? {
        val intent = Intent()
        intent.setAction(Intent.ACTION_OPEN_DOCUMENT)
        intent.addCategory(Intent.CATEGORY_OPENABLE)
        intent.setType("*/*")
        intent.putExtra(Intent.EXTRA_LOCAL_ONLY, true)
        val primaryDirectory = Environment.getExternalStorageDirectory().absolutePath + "/"
        intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, Uri.parse("content://com.android.externalstorage.documents/document/primary%3A${Uri.encode("")}"))
        this@MainActivity.startActivityForResult(intent, REQUEST_PICK_STORAGE_ITEM)
        val targetUri = this.continuation.receive() as Uri?
        return targetUri?.toString()
    }

    private suspend fun pickDirectoryFromDocument(): String? {
        val intent = Intent()
        intent.action = Intent.ACTION_OPEN_DOCUMENT_TREE
        intent.putExtra(Intent.EXTRA_LOCAL_ONLY, true)
        this@MainActivity.startActivityForResult(intent, REQUEST_PICK_STORAGE_ITEM)
        val targetUri = this.continuation.receive() as Uri?
        return targetUri?.toString()
    }


    private suspend fun requestStoragePermission(): Boolean {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            this@MainActivity.requestPermissions(
                arrayOf(
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE
                ),
                REQUEST_REQUEST_EXTERNAL_STORAGE_PERMISSION
            )
        }
        else {
            val intent = Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION, Uri.parse("package:${this@MainActivity.packageName}"))
            this@MainActivity.startActivityForResult(intent, REQUEST_REQUEST_EXTERNAL_STORAGE_PERMISSION)
        }
        this.continuation.receive()
        return this@MainActivity.checkStoragePermission()
    }

    private fun checkStoragePermission(): Boolean {
        val state: Boolean
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            state = this@MainActivity.checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED &&
                    this@MainActivity.checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED
        }
        else {
            state = Environment.isExternalStorageManager()
        }
        return state
    }

    private suspend fun resolveUri(uri: Uri): String? {
        var result: String? = null
        val provider = uri.authority
        var path = uri.path?.let { Uri.decode(it) }
        when (provider) {
            // AOSP DocumentsUI
            "com.android.externalstorage.documents" -> {
                // /document/primary:<path-relative-external-storage>
                if (path?.startsWith("/document/primary:") == true) {
                    result = path.substring("/document/primary:".length)
                    result = "${queryExternalStoragePath()}${if (result.isNullOrEmpty()) "" else "/$result"}"
                }
                // /tree/primary:<path-relative-external-storage>
                if (path?.startsWith("/tree/primary:") == true) {
                    result = path.substring("/tree/primary:".length)
                    result = "${queryExternalStoragePath()}${if (result.isNullOrEmpty()) "" else "/$result"}"
                }
            }
            // Material Files
            "me.zhanghai.android.files.file_provider" -> {
                path = Uri.decode(path ?: "")
                // /file://<path-absolute>
                if (path?.startsWith("/file://") == true) {
                    result = Uri.parse(path.substring(1)).path
                }
            }
            // Root Explorer
            "com.speedsoftware.rootexplorer.fileprovider" -> {
                // /root/<path-relative-root>
                if (path?.startsWith("/root/") == true) {
                    result = path.substring("/root".length)
                }
            }
            // Solid Explorer
            "pl.solidexplorer2.files" -> {
                result = path
            }
            // MT Manager
            "bin.mt.plus.fp" -> {
                result = path
            }
            // NMM
            "in.mfile.files" -> {
                result = path
            }
            else -> {
                if (path != null && path.startsWith("/") && exist(path)) {
                    result = path
                }
            }
        }

        return result
    }

    private suspend fun queryExternalStoragePath(): String {
        return Environment.getExternalStorageDirectory().absolutePath
    }

    private suspend fun exist(path: String): Boolean {
        val file = File(path)
        return file.exists()
    }

}