package com.haruma.sen.environment

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.widget.Toast

class SubActivity : Activity() {

    public override fun onCreate(
        savedInstanceState: Bundle?
    ): Unit {
        super.onCreate(savedInstanceState)
        try {
            val resource = mutableListOf<Uri>()
            when (intent.action) {
                Intent.ACTION_SEND -> {
                    (intent.getParcelableExtra<Uri>(Intent.EXTRA_STREAM))?.let { resource.add(it) }
                }
                Intent.ACTION_SEND_MULTIPLE -> {
                    intent.getParcelableArrayListExtra<Uri>(Intent.EXTRA_STREAM)?.let { resource.addAll(it) }
                }
                Intent.ACTION_VIEW -> {
                    intent.data?.let { resource.add(it) }
                }
            }
            handleResources(resource)
        } catch (e: Exception) {
            showException(e)
        } finally {
            finish()
        }
    }

    private fun handleResources(
        resource: List<Uri>
    ): Unit {
        if (resource.isNotEmpty()) {
            val resultIntent = Intent(this, MainActivity::class.java).apply {
                putParcelableArrayListExtra("resources", ArrayList(resource))
                action = "com.haruma.sen.environment.FORWARD"
                addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP or Intent.FLAG_ACTIVITY_SINGLE_TOP)
            }
            setResult(RESULT_OK, resultIntent)
            startActivity(resultIntent)
        } else {
            setResult(RESULT_CANCELED)
        }
    }

    private fun showException(
        exception: Exception
    ): Unit {
        Toast.makeText(this, exception.message, Toast.LENGTH_LONG).show()
    }
}